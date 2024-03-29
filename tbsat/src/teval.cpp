/*========================================================================
  Copyright (c) 2022 Randal E. Bryant, Carnegie Mellon University

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
========================================================================*/

// Trusted SAT evaluation

/* Enable generation of BDD trace file */
/* #define ENABLE_BTRACE 1 */

#include <ctype.h>
#include <cassert>
#include <algorithm>

#include "tbdd.h"
#include "prover.h"
#include "clause.h"
#include "pseudoboolean.h"

using std::endl;
using std::cout;


using namespace trustbdd;

#define DEFAULT_SEED 123456

// GC Parameters
// Minimum number of dead nodes to trigger GC
#define COLLECT_MIN_LRAT 150000
#define COLLECT_MIN_DRAT  50000
// Minimum fraction of dead:total nodes to trigger GC
#define COLLECT_FRACTION 0.10

// Reporting information
// Give bucket status ~20 times
#define REPORT_BUCKET 20
// Indicate Clause ID every 100 million clauses
#define REPORT_CLAUSE (100*1000*1000)

// Global variables and functions to aid in pseudo-random number generation
Sequencer global_sequencer;

/*
  Generator for phase selection when generating solutions
 */
typedef enum { GENERATE_LOW, GENERATE_HIGH, GENERATE_RANDOM } generator_t;

class PhaseGenerator {
public:

    PhaseGenerator(generator_t gtype, int seed)
    {
        if (gtype == GENERATE_RANDOM)
            seq = new Sequencer(seed);
        else
            seq = NULL;
    }

    PhaseGenerator(PhaseGenerator &pg) {
        gtype = pg.gtype;
        seq = new Sequencer(*(pg.seq));
    }

    ~PhaseGenerator() { if (seq) delete seq; }

    int phase() {
        switch (gtype) {
        case GENERATE_LOW:
            return 0;
        case GENERATE_HIGH:
            return 1;
        default:
            return seq->next() & 0x1;
        }
    }

private:
    Sequencer *seq;
    generator_t gtype;

};


/*
  Represent one step in solution generation following sequence of
  existential quantifications. Solution represented as cube BDD
 */
class Quantification {

public:
    Quantification(ilist vars, bdd lconstraint) {
        variables = ilist_copy(vars);
        ilist_sort(variables);
        local_constraint = lconstraint;
    }

    Quantification(std::vector<int> &vars, bdd lconstraint) {
        variables = ilist_copy_list(vars.data(), vars.size());
        local_constraint = lconstraint;
    }


    ~Quantification() { ilist_free(variables); }

    // Extend partial solution BDD with assignments to quantified variables
    bdd solve_step(bdd solution, PhaseGenerator *pg) {
        // Only need to consider part of local constraint consistent with partial solution
        bdd constraint = bdd_restrict(local_constraint, solution);
        for (int i = ilist_length(variables)-1; i >= 0; i--) {
            int var = variables[i];
            int p = pg->phase();
            bdd litbdd = p ? bdd_ithvar(var) : bdd_nithvar(var);
            bdd nconstraint = bdd_restrict(constraint, litbdd);
            if (nconstraint == bdd_false()) {
                p = !p;
                litbdd = p ? bdd_ithvar(var) : bdd_nithvar(var);
                nconstraint = bdd_restrict(constraint, litbdd);
            }
            constraint = nconstraint;
            solution = bdd_and(litbdd, solution);
            if (verbosity_level >= 3) {
                std::cout << "c Assigned value " << p << " to variable V" << var << std::endl;
            }
        }
        return solution;
    }

    // Impose top-level constraint on this level.  Return resulting existential quantification
    bdd exclude_step(bdd upper_constraint) {
        bdd nlocal_constraint = bdd_and(local_constraint, upper_constraint);
        if (nlocal_constraint == local_constraint)
            return bdd_true();
        if (verbosity_level >= 3) {
            printf("c Imposing new constraint on variables V"); ilist_print(variables, stdout, " V"); printf("\n");
        }
        local_constraint = nlocal_constraint;
        bdd varbdd = bdd_makeset(variables, ilist_length(variables));
        bdd down_constraint = bdd_exist(local_constraint, varbdd);
        return down_constraint;
    }

    // Variables used in quantification
    ilist variables;
    // Local constraint before quantification
    bdd local_constraint;

};

class Solver {
public:

    Solver(PhaseGenerator *pg) {
        phase_gen = pg;
        constraint_function = bdd_true();
    }

    ~Solver() {
        constraint_function = bdd_true();
        for (Quantification *qs : qsteps) delete qs;
    }

    void set_constraint(bdd bfun) {
        constraint_function = bfun;
    }

    void add_step(ilist vars, bdd fun) {
        qsteps.push_back(new Quantification(vars, fun));
    }

    void add_step(std::vector<int> &vars, bdd fun) {
        qsteps.push_back(new Quantification(vars, fun));
    }


    // Generate another solution BDD
    bdd next_solution() {
        if (constraint_function == bdd_false()) return bdd_false();
        bdd solution = bdd_true();
        bdd constraint = bdd_true();
        for (int i = qsteps.size()-1; i >= 0; i--)
            solution = qsteps[i]->solve_step(solution, phase_gen);
        return solution;
    }

    // Impose additional constraint on set of solutions
    void impose_constraint(bdd constraint) {
        for (unsigned i = 0; i < qsteps.size(); i++) {
            constraint = qsteps[i]->exclude_step(constraint);
            if (constraint == bdd_true())
                break;
        }
        constraint_function = bdd_and(constraint_function, constraint);
    }

private:
    PhaseGenerator *phase_gen;
    bdd constraint_function;
    std::vector<Quantification*> qsteps;

};



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
        xor_equation = NULL;
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
    unsigned min_active;
    std::vector<Term *> terms;
    int clause_count;
    int32_t max_variable;
    int verblevel;
    unsigned seed = DEFAULT_SEED;
    proof_type_t proof_type;
    // Estimated total number of nodes
    int total_count;
    // Estimated number of unreachable nodes
    int dead_count;
    // Counters used by proof generator
    int variable_count;
    int last_clause_id;

    // For generating solutions
    Solver *solver;

    // For managing bucket elimination
    // Track which variables have been assigned to a bucket
    std::unordered_set<int> eliminated_variables;

    // Statistics
    int and_count;
    int quant_count;
    int equation_count;
    int max_bdd;

    void check_gc() {
        int collect_min = (proof_type == PROOF_LRAT || proof_type == PROOF_NONE) ? COLLECT_MIN_LRAT : COLLECT_MIN_DRAT;
        if (dead_count >= collect_min && (double) dead_count / total_count >= COLLECT_FRACTION) {
            if (verbosity_level >= 2) {
                std::cout << "c Initiating GC.  Estimated total nodes = " << total_count << ".  Estimated dead nodes = " << dead_count << std::endl;
            }
            bdd_gbc();
            total_count -= dead_count;
            dead_count = 0;
        }
    }

    // Prepare to load with new set of terms
    void reset() {
        next_term_id = 1;
        min_active = 1;
        // Will number terms starting at 1
        terms.resize(1, NULL);
    }

public:

    TermSet(CNF &cnf, FILE *proof_file, ilist variable_ordering, int verb, proof_type_t ptype,
            bool binary, Solver *sol, unsigned s) {
        verblevel = verb;
        seed = s;
        proof_type = ptype;
        tbdd_set_verbose(verb);
        total_count = dead_count = 0;
        clause_count = cnf.clause_count();
        max_variable = cnf.max_variable();
        last_clause_id = clause_count;
        variable_count = max_variable;
        solver = sol;

        ilist *clauses = new ilist[clause_count];
        for (int i = 0; i < clause_count; i++) {
            Clause *cp = cnf[i];
            clauses[i] = cp->data();
        }
        int rcode;
        tbdd_set_verbose(verblevel);
        if ((rcode = tbdd_init(proof_file, &variable_count, &last_clause_id, clauses, variable_ordering, ptype, binary)) != 0) {
            fprintf(stdout, "c Initialization failed.  Return code = %d\n", rcode);
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
        if (verblevel >= 4)
            std::cout << "c Adding term #" << tp->get_term_id() << std::endl;
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
        for (int var : vars) {
            eliminated_variables.insert(var);
        }
        if (solver) solver->add_step(vars, tp->get_root());
        add(new Term(tfun));
        dead_count += tp->deactivate();
        check_gc();
        quant_count++;
        return terms.back();
    }

    Term *equantify(Term *tp, int32_t var) {
        std::vector<int> vars;
        vars.push_back(var);
        return equantify(tp, vars);
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
                dead_count +=  tp1->deactivate();
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

    void show_statistics() {
        bddStat s;
        bdd_stats(s);
        std::cout << "c " << and_count << " conjunctions, " << quant_count << " quantifications." << std::endl;
        std::cout << "c " << equation_count << " equations" << std::endl;
        bdd_printstat();
        std::cout << "c Total BDD nodes: " << s.produced <<std::endl;
        std::cout << "c Max BDD size: " << max_bdd << std::endl;
#if 0
        std::cout << "c Total clauses: " << s.clausenum << std::endl;
        std::cout << "c Max live clauses: " << s.maxclausenum << std::endl;
        std::cout << "c Total variables: " << s.variablenum << std::endl;
#endif
    }

};

bool solve(FILE *cnf_file, int verblevel, bool binary, int max_solutions, unsigned seed) {
    CNF cset = CNF(cnf_file);
    fclose(cnf_file);
    if (cset.failed()) {
        if (verblevel >= 1)
            std::cout << "c Aborted" << std::endl;
        return false;
    }
    if (verblevel >= 1) {
        std::cout << "c Read " << cset.clause_count() << " clauses.  "
                  << cset.max_variable() << " variables" << std::endl;
        std::cout << "c Breaking ties randomly with seed " << seed << std::endl;
    }
    PhaseGenerator pg(GENERATE_RANDOM, DEFAULT_SEED);
    Solver solver(&pg);
    ilist variable_ordering = NULL;
    TermSet tset(cset, NULL, variable_ordering, verblevel, PROOF_NONE, binary, &solver, seed);
    tbdd tr = tbdd_tautology();
    {
        tr = tset.tree_reduce();
        bdd r = tr.get_root();
        std::cout << "c Final BDD size = " << bdd_nodecount(r) << std::endl;
        if (r != bdd_false()) {
            // Enable solution generation
            ilist vlist = ilist_new(1);
            for (int v = 1; v <= cset.max_variable(); v++)
                vlist = ilist_push(vlist, v);
            solver.add_step(vlist, r);
            tr = tbdd_tautology();
            ilist_free(vlist);
        }
    }
    bdd r = tr.get_root();
    if (r == bdd_false())
        std::cout << "s UNSATISFIABLE" << std::endl;
    else {
        std::cout << "s SATISFIABLE" << std::endl;
        cout << "cnt: " << bdd_satcount(r) << endl;
        // Generate solutions
        /* solver.set_constraint(r); */
        /* for (int i = 0; i < max_solutions; i++) { */
        /*     bdd s = solver.next_solution(); */
        /*     if (s == bdd_false()) break; */
        /*     ilist slist = bdd_decode_cube(s); */
        /*     printf("v "); ilist_print(slist, stdout, " "); printf(" 0\n"); */
        /*     ilist_free(slist); */
        /*     // Now exclude this solution from future enumerations. */
        /*     if (i < max_solutions-1) */
        /*         solver.impose_constraint(bdd_not(s)); */
        /* } */
    }
    tbdd_done();
    return true;
}

