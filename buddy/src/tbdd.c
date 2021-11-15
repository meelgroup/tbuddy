#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tbdd.h"
#include "kernel.h"

/*============================================
  Package setup.
============================================*/

/* 
  Set up package.  Arguments:
  - proof output file
  - number of variables in CNF

  When generating LRAT proofs, also require arguments:
  - Number of clauses in CNF
  - The list of clauses, where clause i is at clauses[i-1]
  
  These functions also initialize BuDDy, using parameters tuned according
  to the predicted complexity of the operations.

  Returns 0 if OK, otherwise error code
*/

int tbdd_init_drat(FILE *pfile, int variable_count, int clause_count) {
    return prover_init(pfile, variable_count, clause_count, NULL, false);
}

int tbdd_init_lrat(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses) {
    return prover_init(pfile, variable_count, clause_count, input_clauses, true);
}

void tbdd_set_verbose(int level) {
    verbosity_level = level;
}

/* 
   proof_step = TAUTOLOGY
   root = 1
 */
TBDD tbdd_tautology() {
    TBDD rr;
    rr.root = bdd_true();
    rr.clause_id = TAUTOLOGY;
    return rr;
}

/* 
   proof_step = TAUTOLOGY
   root = 0
 */
TBDD tbdd_null() {
    TBDD rr;
    rr.root = bdd_false();
    rr.clause_id = TAUTOLOGY;
    return rr;
}


/*
  Increment/decrement reference count for BDD
 */
TBDD tbdd_addref(TBDD tr) { bdd_addref(tr.root); return tr; }
void tbdd_delref(TBDD tr) { bdd_delref(tr.root); }

/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.
 */


static BDD bdd_from_clause(ilist clause) {
    int len = ilist_length(clause);
    BDD r = bdd_false();
    int i;
    if (clause == TAUTOLOGY_CLAUSE)
	return r;
    for (i = 0; i < len; i++) {
	int lit = clause[i];
	BDD vr = bdd_addref(lit < 0 ? bdd_nithvar(-lit) : bdd_ithvar(lit));
	BDD nr = bdd_or(r, vr);
	bdd_addref(nr);
	bdd_delref(vr);
	bdd_delref(r);
	r = nr;
    }
    bdd_delref(r);
    return r;
}

TBDD tbdd_from_clause(ilist clause) {
    BDD r = bdd_from_clause(clause);
    return tbdd_trust(r);
}

TBDD tbdd_from_clause_id(int id) {
    TBDD rr;
    ilist clause = get_input_clause(id);
    if (clause == NULL) {
	fprintf(stderr, "Invalid input clause id #%d\n", id);
	exit(1);
    }
    print_proof_comment(2, "Build BDD representation of clause #%d", id);
    BDD r = bdd_addref(bdd_from_clause(clause));
    int len = ilist_length(clause);
    int nlits = 2*len+1;
    int abuf[nlits+ILIST_OVHD];
    ilist ant = ilist_make(abuf, nlits);
    /* Clause literals are in descending order */
    ilist_reverse(clause);
    BDD nd = r;
    int i;
    for (i = 0; i < len; i++) {
	int lit = clause[i];
	if (lit < 0) {
	    ilist_push(ant, bdd_dclause(nd, DEF_LU));
	    ilist_push(ant, bdd_dclause(nd, DEF_HU));
	    nd = bdd_high(nd);
	} else {
	    ilist_push(ant, bdd_dclause(nd, DEF_HU));
	    ilist_push(ant, bdd_dclause(nd, DEF_LU));
	    nd = bdd_low(nd);
	}
    } 
    ilist_push(ant, id);
    int cbuf[1+ILIST_OVHD];
    ilist uclause = ilist_make(cbuf, 1);
    ilist_fill1(uclause, XVAR(r));
    bdd_delref(r);
    rr.root = r;
    print_proof_comment(2, "Validate BDD representation of Clause #%d", id);
    rr.clause_id = generate_clause(uclause, ant);
    return tbdd_trust(r);
}


/*
  Upgrade ordinary BDD to TBDD by proving
  implication from another TBDD
 */
TBDD tbdd_validate(BDD r, TBDD tr) {
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[2+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 2);
    TBDD t = bdd_imptstj(bdd_addref(tr.root), bdd_addref(r));
    bdd_delref(tr.root);
    bdd_delref(r);
    if (t.root != bdd_true()) {
	fprintf(stderr, "Failed to prove implication N%d --> N%d\n", NNAME(tr.root), NNAME(r));
	exit(1);
    }
    print_proof_comment(2, "Validation of unit clause for N%d by implication from N%d",NNAME(r), NNAME(tr.root));
    ilist_fill1(clause, XVAR(r));
    ilist_fill2(ant, t.clause_id, tr.clause_id);
    TBDD rr;
    rr.clause_id = generate_clause(clause, ant);
    rr.root = r;
    return rr;
}

/*
  Declare BDD to be trustworthy.  Proof
  checker must provide validation.
  Only works when generating DRAT proofs
 */
TBDD tbdd_trust(BDD r) {
    TBDD rr;
    int cbuf[2+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 2);
    int abuf[2+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 2);
    print_proof_comment(2, "Assertion of N%d",NNAME(r));
    ilist_fill1(clause, XVAR(r));
    rr.clause_id = generate_clause(clause, ant);
    rr.root = r;
    return rr;
}

/*
  Form conjunction of two TBDDs and prove
  their conjunction implies the new one
 */
TBDD tbdd_and(TBDD tr1, TBDD tr2) {
    TBDD t = bdd_andj(bdd_addref(tr1.root), bdd_addref(tr2.root));
    bdd_delref(tr1.root);
    bdd_delref(tr2.root);
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[3+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 3);
    print_proof_comment(2, "Validate unit clause for node N%d = N%d & N%d", NNAME(t.root), NNAME(tr1.root), NNAME(tr2.root));
    ilist_fill1(clause, XVAR(t.root));
    ilist_fill3(ant, t.clause_id, tr1.clause_id, tr2.clause_id);
    /* Insert proof of unit clause into t's justification */
    t.clause_id = generate_clause(clause, ant);
    return t;
}

/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */
int tbdd_validate_clause(ilist clause, TBDD tr) {
    int abuf[1+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 1);
    ilist_fill1(ant, tr.clause_id);
    print_proof_comment(2, "Fake Validation of clause from N%d", NNAME(tr.root));
    return generate_clause(clause, ant);
}

/*
  Assert that a clause holds.  Proof checker
  must provide validation.
  Use this version when generating DRAT proofs
 */
void assert_clause(ilist clause) {
    int abuf[1+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 1);
    print_proof_comment(2, "Assertion of clause");
    generate_clause(clause, ant);
}

/*============================================
 Useful operations on BDDs
============================================*/

/*
  Build BDD representation of XOR (phase = 1) or XNOR (phase = 0)
 */
BDD bdd_build_xor(ilist variables, int phase) {
    BDD r = phase ? bdd_true() : bdd_false();
    int i;
    for (i = 0; i < ilist_length(variables); i++) {
	BDD lit = bdd_ithvar(variables[i]);
	BDD nr = bdd_xor(r, lit);
	bdd_addref(nr);
	bdd_delref(r);
	r = nr;
    }
    bdd_delref(r);
    return r;
}