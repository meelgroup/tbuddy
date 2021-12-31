// Trusted SAT evaluation

#include <ctype.h>

#include "tbdd.h"
#include "prover.h"
#include "clause.h"
#include "pseudoboolean.h"


using namespace trustbdd;

// GC Parameters

// Minimum number of dead nodes to trigger GC
#define COLLECT_MIN_LRAT 150000
#define COLLECT_MIN_DRAT  20000
// Minimum fraction of dead:total nodes to trigger GC
#define COLLECT_FRACTION 0.20

// Functions to aid parsing of schedule lines

// Skip line.  Return either \n or EOF
static int skip_line(FILE *infile) {
    int c;
    while ((c = getc(infile)) != EOF) {
	if (c == '\n')
	    return c;
    }
    return c;
}

// Skip any whitespace characters other than newline
// Read and return either EOF or first non-space character (possibly newline)
static int skip_space(FILE *infile) {
    int c;
    while ((c = getc(infile)) != EOF) {
	if (c == '\n')
	    return c;
	if (!isspace(c)) {
	    return c;
	}
    }
    return c;
}

// Read rest of line, trimming off leading spaces and trailing newline.
// Return line length
static int get_line(FILE *infile, char *buf, int maxlen) {
    int c = skip_space(infile);
    int pos = 0;
    if (c == EOF || c == '\n') {
	buf[pos] = 0;
	return pos;
    }
    buf[pos++] = c;
    while (true) {
	c = getc(infile);
	if (c == '\n' || c == EOF) {
	    if (pos < maxlen)
		buf[pos] = 0;
	    else
		buf[--pos] = 0;
	    break;
	}
	if (pos < maxlen)
	    buf[pos++] = c;
    }
    return pos;
}


// Read line and parse as set of numbers.  Return either \n or EOF
static int get_numbers(FILE *infile, std::vector<int> &numbers) {
    int c;
    int val;
    numbers.resize(0,0);
    while ((c = getc(infile)) != EOF) {
	if (c == '\n')
	    break;
	if (isspace(c))
	    continue;
	ungetc(c, infile);
	if (fscanf(infile, "%d", &val) != 1) {
	    break;
	}
	numbers.push_back(val);
    }
    return c;
}

// Read line and parse as set of numbers.  Return either \n or EOF
static int get_number_pairs(FILE *infile, std::vector<int> &numbers1, std::vector<int> &numbers2, char sep) {
    int c;
    int val;
    numbers1.resize(0,0);
    numbers2.resize(0,0);
    while ((c = getc(infile)) != EOF) {
	if (c == '\n')
	    break;
	if (isspace(c))
	    continue;
	ungetc(c, infile);
	if (fscanf(infile, "%d", &val) != 1) {
	    break;
	}
	numbers1.push_back(val);
	c = getc(infile);
	if (c != sep) {
	    /** ERROR **/
	    c = 0;
	    break;
	}
	if (fscanf(infile, "%d", &val) != 1) {
	    c = 0;
	    break;
	}
	numbers2.push_back(val);
    }
    return c;
}

static int next_term_id = 1;

class Term {
private:
    int term_id;
    bool is_active;
    tbdd tfun;
    xor_constraint *xor_equation;
    int node_count;

public:
    Term (tbdd t) { 
	term_id = next_term_id++;
	is_active = true; 
	tfun = t;
	node_count = bdd_nodecount(t.get_root());
	xor_equation = NULL;
    }

    // Returns number of dead nodes generated
    int deactivate() {
	tfun = tbdd_null();  // Should delete reference to previous value
	is_active = false;
	int rval = node_count;
	node_count = 0;
	if (xor_equation != NULL)
	    delete xor_equation;
	return rval;
    }

    bool active() {
	return is_active;
    }

    tbdd &get_fun() { return tfun; }

    bdd get_root() { return tfun.get_root(); }

    int get_clause_id() { return tfun.get_clause_id(); }

    xor_constraint *get_equation() { return xor_equation; }

    void set_equation(xor_constraint *eq) { xor_equation = eq; }

    void set_term_id(int val) { term_id = val; }

    int get_term_id() { return term_id; }

    int get_node_count() { return node_count; }

private:

};

class TermSet {
private:
    int min_active;
    std::vector<Term *> terms;
    int clause_count;
    int32_t max_variable;
    int verblevel;
    proof_type_t proof_type;
    // Estimated total number of nodes
    int total_count;
    // Estimated number of unreachable nodes
    int dead_count;
    // Counters used by proof generator
    int variable_count;
    int last_clause_id;
  
    // Statistics
    int and_count;
    int quant_count;
    int equation_count;
    int max_bdd;

    void check_gc() {
	int collect_min = proof_type == PROOF_LRAT ? COLLECT_MIN_LRAT : COLLECT_MIN_DRAT;
	if (dead_count >= collect_min && (double) dead_count / total_count >= COLLECT_FRACTION) {
	    if (verbosity_level >= 2) {
		std::cout << "Initiating GC.  Estimated total nodes = " << total_count << ".  Estimated dead nodes = " << dead_count << std::endl;
	    }
	    bdd_gbc();
	    total_count -= dead_count;
	    dead_count = 0;
	}
    }

    // Load new set of terms
    void reset() {
	next_term_id = 1;
	min_active = 1;
	// Want to number terms starting at 1
	terms.resize(1, NULL);
    }


public:

    TermSet(CNF &cnf, FILE *proof_file, int verb, proof_type_t ptype, bool binary) {
	verblevel = verb;
	proof_type = ptype;
	tbdd_set_verbose(verb);
	total_count = dead_count = 0;
	clause_count = cnf.clause_count();
	max_variable = cnf.max_variable();
	last_clause_id = clause_count;
	variable_count = max_variable;

	ilist *clauses = new ilist[clause_count];
	for (int i = 0; i < clause_count; i++) {
	    Clause *cp = cnf[i];
	    clauses[i] = cp->data();
	}
	int rcode;
	tbdd_set_verbose(verblevel);
	if ((rcode = tbdd_init(proof_file, &variable_count, &last_clause_id, clauses, ptype, binary)) != 0) {
	    fprintf(stderr, "Initialization failed.  Return code = %d\n", rcode);
	    exit(1);
	}
	// Want to number terms starting at 1
	terms.resize(1, NULL);
	for (int i = 1; i <= clause_count; i++) {
	    tbdd tc = tbdd_from_clause_id(i);
	    add(new Term(tc));
	}
	min_active = 1;
	and_count = 0;
	quant_count = 0;
	equation_count = 0;
	max_bdd = 0;
    }
  
    void add(Term *tp) {
	tp->set_term_id(terms.size());
	max_bdd = std::max(max_bdd, bdd_nodecount(tp->get_root()));
	terms.push_back(tp);
	//    if (verblevel >= 3) 
	//      std::cout << "Adding term #" << tp->get_term_id() << std::endl;
	total_count += tp->get_node_count();
    }

    Term *conjunct(Term *tp1, Term *tp2) {
	tbdd tr1 = tp1->get_fun();
	tbdd tr2 = tp2->get_fun();
	tbdd nfun = tbdd_and(tr1, tr2);
	add(new Term(nfun));
	dead_count += tp1->deactivate();
	dead_count += tp2->deactivate();
	check_gc();
	and_count++;
	return terms.back();
    }

    Term *equantify(Term *tp, std::vector<int> &vars) {
	int *varset = vars.data();
	bdd varbdd = bdd_makeset(varset, vars.size());
	bdd nroot = bdd_exist(tp->get_root(), varbdd);
	tbdd tfun = tbdd_validate(nroot, tp->get_fun());
	add(new Term(tfun));
	dead_count += tp->deactivate();
	check_gc();
	quant_count++;
	return terms.back();
    }

    Term *equantify(Term *tp, int32_t var) {
	bdd varbdd = bdd_ithvar(var);
	bdd nroot = bdd_exist(tp->get_root(), varbdd);
	tbdd tfun = tbdd_validate(nroot, tp->get_fun());
	add(new Term(tfun));
	dead_count += tp->deactivate();
	check_gc();
	quant_count++;
	return terms.back();
    }

    Term *xor_constrain(Term *tp, std::vector<int> &vars, int constant) {
	ilist variables = ilist_copy_list(vars.data(), vars.size());
	xor_constraint *xor_equation = new xor_constraint(variables, constant, tp->get_fun());
	Term *tpn = new Term(xor_equation->get_validation());
	tpn->set_equation(xor_equation);
	add(tpn);
	dead_count += tp->deactivate();
	check_gc();
	equation_count++;
	return terms.back();
    }

    // Form conjunction of terms until reduce to <= 1 term
    // Effectively performs a tree reduction
    // Return final bdd
    tbdd tree_reduce() {
	Term *tp1, *tp2;
	while (true) {
	    while (min_active < terms.size() && !terms[min_active]->active())
		min_active++;
	    if (min_active >= terms.size())
		// Didn't find any terms.  Formula is tautology
		return tbdd_tautology();
	    tp1 = terms[min_active++];
	    while (min_active < terms.size() && !terms[min_active]->active())
		min_active++;
	    if (min_active >= terms.size()) {
		// There was only one term left
		tbdd result = tp1->get_fun();
		tp1->deactivate();
		return result;
	    }
	    tp2 = terms[min_active++];
	    Term *tpn = conjunct(tp1, tp2);
	    if (tpn->get_root() == bdd_false()) {
		tbdd result = tpn->get_fun();
		return result;
	    }
	}
    }


    tbdd bucket_reduce() {
	std::vector<int> *buckets = new std::vector<int>[max_variable+1];
	int tcount = 0;
	int bcount = 0;
	for (int i = min_active; i < terms.size(); i++) {
	    Term *tp = terms[i];
	    if (!tp->active())
		continue;
	    bdd root = tp->get_root();
	    if (root == bdd_false()) {
		// Formula is trivially false
		tbdd result = tp->get_fun();
		return result;
	    }
	    if (root != bdd_true()) {
		int top = bdd_var(root);
		if (buckets[top].size() == 0)
		    bcount++;
		buckets[top].push_back(i);
		tcount++;
	    }
	}
	if (verblevel >= 1)
	    std::cout << "Placed " << tcount << " terms into " << bcount << " buckets." << std::endl;

	for (int bvar = 1 ; bvar <= max_variable; bvar++) {
	    int next_idx = 0;
	    if (buckets[bvar].size() == 0) {
		if (verblevel >= 3)
		    std::cout << "Bucket " << bvar << " empty.  Skipping" << std::endl;
		continue;
	    }
	    while (next_idx < buckets[bvar].size() - 1) {
		Term *tp1 = terms[buckets[bvar][next_idx++]];
		Term *tp2 = terms[buckets[bvar][next_idx++]];
		Term *tpn = conjunct(tp1, tp2);
		bdd root = tpn->get_root();
		if (root == bdd_false()) {
		    if (verblevel >= 3)
			std::cout << "Bucket " << bvar << " Conjunction of terms " 
				  << tp1->get_term_id() << " and " << tp2->get_term_id() << " yields FALSE" << std::endl;
		    tbdd result = tpn->get_fun();
		    return result;
		}
		int top = bdd_var(root);
		if (verblevel >= 3)
		    std::cout << "Bucket " << bvar << " Conjunction of terms " 
			      << tp1->get_term_id() << " and " << tp2->get_term_id() << " yields term " 
			      << tpn->get_term_id() << " with top variable " << top << std::endl;
		buckets[top].push_back(tpn->get_term_id());
	    }
	    if (next_idx == buckets[bvar].size()-1) {
		Term *tp = terms[buckets[bvar][next_idx]];
		Term *tpn = equantify(tp, bvar);
		bdd root = tpn->get_root();
		if (verblevel >= 1 && bvar % 100 == 0)
		    std::cout << "Bucket " << bvar << " Reduced to term with " << tpn->get_node_count() << " nodes" << std::endl;
		if (root == bdd_true()) {
		    if (verblevel >= 3)
			std::cout << "Bucket " << bvar << " Quantification of term " 
				  << tp->get_term_id() << " yields TRUE" << std::endl;
		} else {
		    int top = bdd_var(root);
		    buckets[top].push_back(tpn->get_term_id());
		    if (verblevel >= 3) {
			std::cout << "Bucket " << bvar << " Quantification of term " 
				  << tp->get_term_id() << " yields term " << tpn->get_term_id() 
				  << " with top variable " << top << std::endl;
		    }

		}
	    }
	}
	// If get here, formula must be satisfiable
	if (verblevel >= 1) {
	    std::cout << "Tautology" << std::endl;
	}
	return tbdd_tautology();
    }

    tbdd schedule_reduce(FILE *schedfile) {
	int line = 1;
	int modulus = INT_MAX;
	int constant;
	int i;
	std::vector<Term *> term_stack;
	std::vector<int> numbers;
	std::vector<int> numbers2;
	while (true) {
	    int c;
	    if ((c = skip_space(schedfile)) == EOF)
		break;
	    switch(c) {
	    case '\n':
		line++;
		break;
	    case '#':
		c = skip_line(schedfile);
		line++;
		break;
	    case 'i': 
		if (term_stack.size() > 0 && verblevel > 0) {
		    char buf[1024];
		    int len = get_line(schedfile, buf, 1024);
		    Term *tp = term_stack.back();
		    int term_id = tp->get_term_id();
		    bdd root = tp->get_root();
		    std::cout << "Term #" << term_id << ". Nodes = " << bdd_nodecount(root) << ". " << buf << std::endl;
		}
		break;
	    case 'c':
		c = get_numbers(schedfile, numbers);
		if (c != '\n' && c != EOF) {
		    fprintf(stderr, "Schedule line #%d.  Clause command. Non-numeric argument '%c'\n", line, c);
		    exit(1);
		}
		for (int i = 0; i < numbers.size(); i++) {
		    int ci = numbers[i];
		    if (ci < 1 || ci > clause_count) {
			fprintf(stderr, "Schedule line #%d.  Invalid clause number %d\n", line, ci);
			exit(1);
		    }
		    if (ci >= terms.size()) {
			fprintf(stderr, "Schedule line #%d.  Internal error.  Attempting to get clause #%d, but only have %d terms\n", line, ci, (int) terms.size()-1);
			exit(1);
		    }
		    term_stack.push_back(terms[ci]);
		}
		if (verblevel >= 3) {
		    std::cout << "Schedule line #" << line << ".  Pushed " << numbers.size() 
			      << " clauses.  Stack size = " << term_stack.size() << std::endl;
		}
		line ++;
		break;
	    case 'a':
		c = get_numbers(schedfile, numbers);
		if (c != '\n' && c != EOF) {
		    fprintf(stderr, "Schedule line #%d.  And command. Non-numeric argument '%c'\n", line, c);
		    exit(1);
		}
		if (numbers.size() != 1) {
		    fprintf(stderr, "Schedule line #%d.  Should specify number of conjunctions\n", line);
		    exit(1);
		} else {
		    int ccount = numbers[0];
		    if (ccount < 1 || ccount > term_stack.size()-1) {
			fprintf(stderr, 
				"Schedule line #%d.  Cannot perform %d conjunctions.  Stack size = %d\n",
				line, ccount, (int) term_stack.size());
			exit(1);
		    }
		    Term *product = term_stack.back();
		    term_stack.pop_back();
		    if (!product->active()) {
			fprintf(stderr, "Schedule line #%d.  Attempting to reuse clause #%d\n", line, product->get_term_id());
			exit(1);
		    }
		    while (ccount-- > 0) {
			Term *tp = term_stack.back();
			term_stack.pop_back();
			if (!tp->active()) {
			    fprintf(stderr, "Schedule line #%d.  Attempting to reuse clause #%d\n", line, tp->get_term_id());
			    exit(1);
			}
			product = conjunct(product, tp);
			if (product->get_root() == bdd_false()) {
			    if (verblevel >= 2) {
				std::cout << "Schedule line #" << line << ".  Generated BDD 0" << std::endl;
			    }
			    tbdd result = product->get_fun();
			    return result;
			}
		    }
		    term_stack.push_back(product);
		    if (verblevel >= 3) {
			std::cout << "Schedule line #" << line << ".  Performed " << numbers[0]
				  << " conjunctions to get term #" << product->get_term_id() << ".  Stack size = " << term_stack.size() << std::endl;
		    }
		}
		line ++;
		break;
	    case 'q':
		c = get_numbers(schedfile, numbers);
		if (c != '\n' && c != EOF) {
		    fprintf(stderr, "Schedule line #%d.  Quantify command. Non-numeric argument '%c'\n", line, c);
		    exit(1);
		}
		for (int i = 0; i < numbers.size(); i++) {
		    int vi = numbers[i];
		    if (vi < 1 || vi > max_variable) {
			fprintf(stderr, "Schedule line #%d.  Invalid variable %d\n", line, vi);
			exit(1);
		    }
		}
		if (term_stack.size() < 1) {
		    fprintf(stderr, "Schedule line #%d.  Cannot quantify.  Stack is empty\n", line);
		    exit(1);
		} else {
		    Term *tp = term_stack.back();
		    term_stack.pop_back();
		    Term *tpn = equantify(tp, numbers);
		    term_stack.push_back(tpn);
		    if (verblevel >= 3) {
			std::cout << "Schedule line #" << line << ".  Quantified " << numbers.size()
				  << " variables to get Term #" << tpn->get_term_id() << ".  Stack size = " << term_stack.size() << std::endl;
		    }
		}
		line ++;
		break;
	    case '=':
		// See if there's a modulus 
		c = getc(schedfile);
		if (isdigit(c)) {
		    ungetc(c, schedfile);
		    if (fscanf(schedfile, "%d", &modulus) != 1) {
			fprintf(stderr, "Schedule line #%d.  Invalid modulus\n", line);
			exit(1);
		    } else if (modulus != 2) {
			fprintf(stderr, "Schedule line #%d.  Only support modulus 2\n", line);
			exit(1);
		    }
		} else {
		    fprintf(stderr, "Schedule line #%d.  Modulus required\n", line);
		    exit(1);
		}
		if (fscanf(schedfile, "%d", &constant) != 1) {
		    fprintf(stderr, "Schedule line #%d.  Constant term required\n", line);
		    exit(1);
		}
		if (constant < 0 || constant >= modulus) {
		    fprintf(stderr, "Schedule line #%d.  Constant term %d invalid.  Must be between 0 and %d\n", line, constant, modulus-1);
		    exit(1);
		}
		c = get_number_pairs(schedfile, numbers2, numbers, '.');
		if (c != '\n' && c != EOF) {
		    fprintf(stderr, "Schedule line #%d.  Could not parse equation terms\n", line);
		    exit(1);
		}
		for (i = 0; i < numbers2.size(); i++) {
		    int coeff = numbers2[i];
		    if (coeff != 1) {
			fprintf(stderr, "Schedule line #%d.  Invalid coefficient %d\n", line, coeff);
			exit(1);
		    }
		}
		if (term_stack.size() < 1) {
		    fprintf(stderr, "Schedule line #%d.  Cannot extract equation.  Stack is empty\n", line);
		    exit(1);
		} else {
		    Term *tp = term_stack.back();
		    term_stack.pop_back();
		    Term *tpn = xor_constrain(tp, numbers, constant);
		    term_stack.push_back(tpn);
		    if (verblevel >= 3) {
			std::cout << "Schedule line #" << line << ".  Xor constraint with " << numbers.size()
				  << " variables to get Term #" << tpn->get_term_id() <<  ".  Stack size = " << term_stack.size() << std::endl;
		    }
		}
		line ++;
		break;
	    case 'g':
		c = get_numbers(schedfile, numbers);
		if (c != '\n' && c != EOF) {
		    fprintf(stderr, "Schedule line #%d.  Gauss command. Non-numeric argument '%c'\n", line, c);
		    exit(1);
		}
		if (numbers.size() < 1) {
		    fprintf(stderr, "Schedule line #%d.  Should specify number of equations to sum\n", line);
		    exit(1);
		} else {
		    int ecount = numbers[0];
		    if (ecount < 1 || ecount > term_stack.size()) {
			fprintf(stderr, 
				"Schedule line #%d.  Cannot perform Gaussian elimination on %d equations.  Stack size = %d\n",
				line, ecount, (int) term_stack.size());
			exit(1);
		    }
		    ilist exvars = ilist_new(numbers.size()-1);
		    for (int i = 1; i < numbers.size(); i++)
			ilist_push(exvars, numbers[i]);
		    xor_set xset;
		    for (i = 0; i < ecount; i++) {
			int si = term_stack.size() - i - 1;
			Term *tp = term_stack[si];
			if (tp->get_equation() == NULL) {
			    fprintf(stderr, "Schedule line #%d.  Term %d does not have an associated equation\n", line, tp->get_term_id());
			    exit(1);
			}
			xset.add(*tp->get_equation());
		    }
		    xor_set nset;
		    xset.gauss_jordan(exvars, nset);
		    ilist_free(exvars);
		    if (nset.is_infeasible()) {
			if (verblevel >= 2) {
			    std::cout << "Schedule line #" << line << ".  Generated infeasible constraint" << std::endl;
			}
			tbdd result = nset.xlist[0]->get_validation();
			return result;
		    }
		    for (i = 0; i < ecount; i++) {
			Term *tp = term_stack.back();
			term_stack.pop_back();
			dead_count += tp->deactivate();
		    }
		    if (nset.xlist.size() == 0) {
			if (verblevel >= 3) {
			    std::cout << "Schedule line #" << line << ".  G-J elim on  " << ecount << 
				" equations gives no new terms.  Stack size = " << term_stack.size() << std::endl;
			}
		    } else {
			int first_term = -1;
			int last_term = -1;
			for (xor_constraint *xc : nset.xlist) {
			    Term *tpn = new Term(xc->get_validation());
			    last_term = tpn->get_term_id();
			    if (first_term < 0)
				first_term = last_term;
			    term_stack.push_back(tpn);
			}
			nset.clear();
			check_gc();
			if (verblevel >= 3) {
			    std::cout << "Schedule line #" << line << ".  G-J elim on " << ecount << 
				" equations gives Terms #" << first_term << "--#" << last_term << ".  Stack size = " << term_stack.size() << std::endl;
			}
		    }
		}
		line++;
		break;
	    default:
		fprintf(stderr, "Schedule line #%d.  Unknown command '%c'\n", line, c);
		break;
	    }
	}
	if (term_stack.size() != 1) {
	    if (verblevel >= 2)
		std::cout << "After executing schedule, have " << term_stack.size() << " terms.  Switching to bucket elimination" << std::endl;
	    // Hack things up to treat remaining terms as entire set
	    reset();
	    for (Term *tp : term_stack) {
		add(new Term(tp->get_fun()));
	    }
	    return bucket_reduce();

	}
	Term *tp = term_stack.back();
	return tp->get_fun();
    }

    void show_statistics() {
	bddStat s;
	bdd_stats(s);
	std::cout << and_count << " conjunctions, " << quant_count << " quantifications." << std::endl;
	std::cout << equation_count << " equations" << std::endl;
	bdd_printstat();
	std::cout << "Total BDD nodes: " << s.produced <<std::endl;
	std::cout << "Max BDD size: " << max_bdd << std::endl;
	std::cout << "Total clauses: " << s.clausenum << std::endl;
	std::cout << "Max live clauses: " << s.maxclausenum << std::endl;
	std::cout << "Total variables: " << s.variablenum << std::endl;
    }

};

bool solve(FILE *cnf_file, FILE *proof_file, FILE *sched_file, bool bucket, int verblevel, proof_type_t ptype, bool binary) {
    CNF cset = CNF(cnf_file);
    fclose(cnf_file);
    if (cset.failed()) {
	if (verblevel >= 1)
	    std::cout << "Aborted" << std::endl;
	return false;
    }
    if (verblevel >= 1)
	if (verblevel >= 1)
	    std::cout << "Read " << cset.clause_count() << " clauses.  " 
		      << cset.max_variable() << " variables" << std::endl;
    TermSet tset = TermSet(cset, proof_file, verblevel, ptype, binary);
    tbdd tr = tbdd_tautology();
    if (sched_file != NULL)
	tr = tset.schedule_reduce(sched_file);
    else if (bucket)
	tr = tset.bucket_reduce();
    else
	tr = tset.tree_reduce();
    bdd r = tr.get_root();
    if (r == bdd_true())
	std::cout << "TAUTOLOGY" << std::endl;
    else if (r == bdd_false())
	std::cout << "UNSATISFIABLE" << std::endl;
    else {
	std::cout << "Satisfiable.  BDD size = " << bdd_nodecount(r) << std::endl;
	if (verblevel >= 3)
	    std::cout << "BDD: " << r << std::endl;
    }
    if (verblevel >= 1)
	tset.show_statistics();
    //  std::cout << "Running garbage collector" << std::endl;
    //  bdd_gbc();
    bdd_done();
    return true;
}

