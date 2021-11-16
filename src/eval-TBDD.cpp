// This is a hacked up version of the program to avoid the C++ interface to Buddy.

// Macros to return integer values
#define BDD_TRUE 1
#define BDD_FALSE 0

#include <ctype.h>

#include "tbdd.h"
#include "prover.h"
#include "clause.h"


// Buddy parameters
// Cutoff betweeen large and small allocations (in terms of clauses)
#define BUDDY_THRESHOLD 1000
//#define BUDDY_THRESHOLD 10
#define BUDDY_NODES_LARGE (1000*1000)
//#define BUDDY_NODES_LARGE (10*1000)
#define BUDDY_NODES_SMALL (    100*1000)
#define BUDDY_CACHE_RATIO 8
#define BUDDY_INCREASE_RATIO 20

// GC Parameters

// Minimum number of dead nodes to trigger GC
#define COLLECT_MIN 50000
// Maximum number of nodes to keep around without GC
#define COLLECT_MAX 500000
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
static int getline(FILE *infile, char *buf, int maxlen) {
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

static int next_id = 1;

class Term {
private:
  int id;
  bool is_active;
  TBDD tfun;
  int node_count;

public:
  Term (TBDD t) { 
    id = next_id++;
    is_active = true; 
    tfun = t;
    bdd_addref(t.root);
    node_count = bdd_nodecount(t.root);
  }

  // Returns number of dead nodes generated
  int deactivate() {
    bdd_delref(tfun.root);
    is_active = false;
    int rval = node_count;
    node_count = 0;
    return rval;
  }

  bool active() {
    return is_active;
  }

  TBDD get_fun() { return tfun; }

  BDD get_root() { return tfun.root; }

  int get_clause_id() { return tfun.clause_id; }

  void set_id(int val) { id = val; }

  int get_id() { return id; }

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
  // Estimated total number of nodes
  int total_count;
  // Estimated number of unreachable nodes
  int dead_count;
  
  // Statistics
  int and_count;
  int quant_count;
  int max_bdd;

  void check_gc() {
    if (dead_count >= COLLECT_MAX 
	|| (dead_count >= COLLECT_MIN && (double) dead_count / total_count >= COLLECT_FRACTION)) {
      if (verbosity_level >= 2) {
	std::cout << "Initiating GC.  Estimated total nodes = " << total_count << ".  Estimated dead nodes = " << dead_count << std::endl;
      }
      bdd_gbc();
      total_count -= dead_count;
      dead_count = 0;
    }
  }

public:

  TermSet(CNF &cnf, FILE *proof_file, int verb) {
    verblevel = verb;
    tbdd_set_verbose(verb);
    total_count = dead_count = 0;
    clause_count = cnf.clause_count();
    max_variable = cnf.max_variable();

    ilist *clauses = new ilist[clause_count];
    for (int i = 0; i < clause_count; i++) {
      Clause *cp = cnf[i];
      clauses[i] = ilist_copy_list(cp->data(), cp->length());
#if 0
      printf("Input clause #%d: [", i+1);
      ilist_print(clauses[i], stdout, (char *) " ");
      printf("]\n");
#endif
    }
    int rcode;
    if ((rcode = tbdd_init_lrat(proof_file, max_variable, clause_count, clauses)) != 0) {
      fprintf(stderr, "Initialization failed.  Return code = %d\n", rcode);
      exit(1);
    }
    /* Can free storage allocated for clauses */
    for (int i = 0; i < clause_count; i++) {
      ilist_free(clauses[i]);
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
    max_bdd = 0;
  }
  
  void add(Term *tp) {
    tp->set_id(terms.size());
    max_bdd = std::max(max_bdd, bdd_nodecount(tp->get_root()));
    terms.push_back(tp);
    if (verblevel >= 3) 
      std::cout << "Adding term #" << tp->get_id() << std::endl;
    total_count += tp->get_node_count();
  }

  Term *conjunct(Term *tp1, Term *tp2) {
    TBDD tr1 = tp1->get_fun();
    TBDD tr2 = tp2->get_fun();
    TBDD nfun;
    if (tbdd_is_false(tr1))
      nfun = tr1;
    else if (tbdd_is_false(tr2))
      nfun = tr2;
    else
      nfun = tbdd_and(tr1, tr2);
    add(new Term(nfun));
    dead_count += tp1->deactivate();
    dead_count += tp2->deactivate();
    check_gc();
    and_count++;
    return terms.back();
  }

  Term *equantify(Term *tp, std::vector<int> &vars) {
    int *varset = vars.data();
    BDD varbdd = bdd_addref(BDD_makeset(varset, vars.size()));
    BDD nroot = bdd_addref(bdd_exist(tp->get_root(), varbdd));
    TBDD tfun = tbdd_validate(nroot, tp->get_fun());
    add(new Term(tfun));
    bdd_delref(varbdd);
    bdd_delref(nroot);
    dead_count += tp->deactivate();
    check_gc();
    quant_count++;
    return terms.back();
  }

  Term *equantify(Term *tp, int32_t var) {
    BDD varbdd = bdd_addref(BDD_ithvar(var));
    BDD nroot = bdd_addref(bdd_exist(tp->get_root(), varbdd));
    TBDD tfun = tbdd_validate(nroot, tp->get_fun());
    add(new Term(tfun));
    bdd_delref(varbdd);
    bdd_delref(nroot);
    dead_count += tp->deactivate();
    check_gc();
    quant_count++;
    return terms.back();
  }

  void flush() {
    while (min_active < terms.size()) {
      Term *tp = terms[min_active];
      if (tp->active()) {
	if (verblevel >= 3) 
	  std::cout << "Flushing term #" << tp->get_id() << std::endl;
	tp->deactivate();
      }
      min_active++;
    }
  }

  // Form conjunction of terms until reduce to <= 1 term
  // Effectively performs a tree reduction
  // Return final bdd
  TBDD tree_reduce() {
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
	TBDD result = tp1->get_fun();
	tp1->deactivate();
	return result;
      }
      tp2 = terms[min_active++];
      Term *tpn = conjunct(tp1, tp2);
      if (tpn->get_root() == BDD_FALSE) {
	TBDD result = tpn->get_fun();
	flush();
	return result;
      }
    }
  }

  TBDD bucket_reduce() {
    std::vector<int> *buckets = new std::vector<int>[max_variable+1];
    int tcount = 0;
    int bcount = 0;
    for (int i = min_active; i < terms.size(); i++) {
      Term *tp = terms[i];
      if (!tp->active())
	continue;
      BDD root = tp->get_root();
      if (root == BDD_FALSE) {
	// Formula is trivially false
	TBDD result = tp->get_fun();
	flush();
	return result;
      }
      if (root != BDD_TRUE) {
	int top = bdd_var(root);
	if (buckets[top].size() == 0)
	  bcount++;
	buckets[top].push_back(tp->get_id());
	tcount++;
      }
    }
    if (verblevel >= 1)
      std::cout << "Placed " << tcount << " terms into " << bcount << " buckets." << std::endl;

    for (int bvar = 1 ; bvar <= max_variable; bvar++) {
      while (buckets[bvar].size() > 1) {
	Term *tp1 = terms[buckets[bvar].back()];
	buckets[bvar].pop_back();
	Term *tp2 = terms[buckets[bvar].back()];
	buckets[bvar].pop_back();
	Term *tpn = conjunct(tp1, tp2);
	BDD root = tpn->get_root();
	if (root == BDD_FALSE) {
	  if (verblevel >= 3)
	    std::cout << "Bucket " << bvar << " Conjunction of terms " 
		      << tp1->get_id() << " and " << tp2->get_id() << " yields FALSE" << std::endl;
	  TBDD result = tpn->get_fun();
	  flush();
	  return result;
	}
	int top = bdd_var(root);
	if (verblevel >= 3)
	  std::cout << "Bucket " << bvar << " Conjunction of terms " 
		    << tp1->get_id() << " and " << tp2->get_id() << " yields term " 
		    << tpn->get_id() << " with top variable " << top << std::endl;
	buckets[top].push_back(tpn->get_id());
      }
      if (buckets[bvar].size() == 1) {
	Term *tp = terms[buckets[bvar].back()];
	buckets[bvar].pop_back();
	Term *tpn = equantify(tp, bvar);
	BDD root = tpn->get_root();
	if (root == BDD_TRUE) {
	  if (verblevel >= 3)
	    std::cout << "Bucket " << bvar << " Quantification of term " 
		      << tp->get_id() << " yields TRUE" << std::endl;
	} else {
	  int top = bdd_var(root);
	  buckets[top].push_back(tpn->get_id());
	  if (verblevel >= 3)
	    std::cout << "Bucket " << bvar << " Quantification of term " 
		      << tp->get_id() << " yields term " << tpn->get_id() 
		      << " with top variable " << top << std::endl;
	}
      }
    }
    // If get here, formula must be satisfiable
    return tbdd_tautology();
  }

  TBDD schedule_reduce(FILE *schedfile) {
    int line = 1;
    std::vector<Term *> term_stack;
    std::vector<int> numbers;
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
	  int len = getline(schedfile, buf, 1024);
	  Term *tp = term_stack.back();
	  int id = tp->get_id();
	  BDD root = tp->get_root();
	  std::cout << "Term #" << id << ". Nodes = " << bdd_nodecount(root) << ". " << buf << std::endl;
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
	    fprintf(stderr, "Schedule line #%d.  Attempting to reuse clause #%d\n", line, product->get_id());
	    exit(1);
	  }
	  while (ccount-- > 0) {
	    Term *tp = term_stack.back();
	    term_stack.pop_back();
	    if (!tp->active()) {
	      fprintf(stderr, "Schedule line #%d.  Attempting to reuse clause #%d\n", line, tp->get_id());
	      exit(1);
	    }
	    product = conjunct(product, tp);
	    if (product->get_root() == BDD_FALSE) {
	      if (verblevel >= 2) {
		std::cout << "Schedule line #" << line << ".  Generated BDD 0" << std::endl;
	      }
	      TBDD result = product->get_fun();
	      flush();
	      return result;
	    }
	  }
	  term_stack.push_back(product);
	}
	if (verblevel >= 3) {
	  std::cout << "Schedule line #" << line << ".  Performed " << numbers[0]
		    << " conjunctions.  Stack size = " << term_stack.size() << std::endl;
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
	}
	if (verblevel >= 3) {
	  std::cout << "Schedule line #" << line << ".  Quantified " << numbers.size()
		    << " variables.  Stack size = " << term_stack.size() << std::endl;
	}
	line ++;
	break;
      default:
	fprintf(stderr, "Schedule line #%d.  Unknown command '%c'\n", line, c);
	break;
      }
    }
    if (term_stack.size() != 1) {
      fprintf(stderr, "Schedule line #%d.  Schedule ended with %d terms on stack\n", line, (int) term_stack.size());
      exit(1);
    }
    Term *tp = term_stack.back();
    return tp->get_fun();
  }

  void show_statistics() {
    bddStat s;
    bdd_stats(s);
    std::cout << and_count << " conjunctions, " << quant_count << " quantifications." << std::endl;
    bdd_printstat();
    std::cout << "Total BDD nodes: " << s.produced <<std::endl;
    std::cout << "Max BDD size: " << max_bdd << std::endl;
    std::cout << "Total clauses: " << s.clausenum << std::endl;
    std::cout << "Total variables: " << s.variablenum << std::endl;
  }

};

bool solve(FILE *cnf_file, FILE *proof_file, FILE *sched_file, bool bucket, int verblevel) {
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
  TermSet tset = TermSet(cset, proof_file, verblevel);
  tbdd tr;
  if (sched_file != NULL)
    tr = tset.schedule_reduce(sched_file);
  else if (bucket)
    tr = tset.bucket_reduce();
  else
    tr = tset.tree_reduce();
  BDD r = tr.root;
  if (r == BDD_TRUE)
    std::cout << "TAUTOLOGY" << std::endl;
  else if (r == BDD_FALSE)
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

