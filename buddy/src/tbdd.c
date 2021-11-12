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
tbdd tbdd_tautology() {
    tbdd rr;
    rr.root = bdd_true();
    rr.clause_id = TAUTOLOGY;
    return rr;
}

/*
  Increment/decrement reference count for BDD
 */
tbdd tbdd_addref(tbdd tr) { bdd_addref(tr.root); return tr; }
void tbdd_delref(tbdd tr) { bdd_delref(tr.root); }

/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.
 */


static BDD bdd_from_clause(ilist clause) {
    int len = ilist_length(clause);
    bdd r = bdd_false();
    int i;
    if (clause == TAUTOLOGY_CLAUSE)
	return r;
    for (i = 0; i < len; i++) {
	int lit = clause[i];
	bdd vr = bdd_addref(lit < 0 ? bdd_nithvar(-lit) : bdd_ithvar(lit));
	bdd nr = bdd_or(r, vr);
	bdd_addref(nr);
	bdd_delref(vr);
	bdd_delref(r);
	r = nr;
    }
    bdd_delref(r);
    return r;
}

tbdd tbdd_from_clause(ilist clause) {
    bdd r = bdd_from_clause(clause);
    return tbdd_trust(r);
}

tbdd tbdd_from_clause_id(int id) {
    tbdd rr;
    ilist clause = get_input_clause(id);
    if (clause == NULL) {
	fprintf(stderr, "Invalid input clause id #%d\n", id);
	exit(1);
    }
    print_proof_comment(2, "Build BDD representation of clause #%d", id);
    bdd r = bdd_addref(bdd_from_clause(clause));
    int len = ilist_length(clause);
    int nlits = 2*len;
    int abuf[nlits+ILIST_OVHD];
    ilist ant = ilist_make(abuf, nlits);
    /* Clause literals are in descending order */
    ilist_reverse(clause);
    bdd nd = r;
    int i;
    for (i = 0; i < len; i++) {
	int lit = clause[i];
	int ida, idb;
	if (lit < 0) {
	    ida = bdd_dclause(nd, DEF_LU);
	    idb = bdd_dclause(nd, DEF_HU);
	    nd = bdd_high(r);
	} else {
	    ida = bdd_dclause(nd, DEF_HU);
	    idb = bdd_dclause(nd, DEF_LU);
	    nd = bdd_low(r);
	}
	if (ida != TAUTOLOGY)
	    ilist_push(ant, ida);
	if (idb != TAUTOLOGY)
	    ilist_push(ant, idb);
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
tbdd tbdd_validate(bdd r, tbdd tr) {
    tbdd rr;
    int cbuf[2+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 2);
    int abuf[2+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 2);
    ilist_fill2(clause, XVAR(r), -XVAR(tr.root));
    print_proof_comment(2, "Proof of implication N%d --> N%d", NNAME(tr.root), NNAME(r));
    int reason = generate_clause(clause, ant);
    print_proof_comment(2, "Justification of N%d",NNAME(r));
    ilist_fill1(clause, XVAR(r));
    ilist_fill2(ant, reason, tr.clause_id);
    rr.clause_id = generate_clause(clause, ant);
    rr.root = r;
    return rr;
}

/*
  Declare BDD to be trustworthy.  Proof
  checker must provide validation.
  Only works when generating DRAT proofs
 */
tbdd tbdd_trust(bdd r) {
    tbdd rr;
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
tbdd tbdd_and(tbdd tr1, tbdd tr2) {
    tbdd rr;
    bdd nr = bdd_and(bdd_addref(tr1.root), bdd_addref(tr2.root));
    int cbuf[3+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 3);
    int abuf[3+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 3);
    print_proof_comment(2, "Proof that N%d & N%d --> N%d", NNAME(tr1.root), NNAME(tr2.root), NNAME(nr));
    ilist_fill3(clause, -XVAR(tr1.root), -XVAR(tr2.root), XVAR(nr));
    int implication = generate_clause(clause, ant);
    print_proof_comment(2, "Justification of N%d",NNAME(nr));
    ilist_fill1(clause, XVAR(nr));
    ilist_fill3(ant, implication, tr1.clause_id, tr2.clause_id);
    rr.clause_id = generate_clause(clause, ant);
    rr.root = nr;
    bdd_delref(tr1.root);
    bdd_delref(tr2.root);
    return rr;
}

/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */
int tbdd_validate_clause(ilist clause, tbdd tr) {
    int abuf[1+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 1);
    ilist_fill1(ant, tr.clause_id);
    print_proof_comment(2, "Validation of clause from N%d", NNAME(tr.root));
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
bdd bdd_build_xor(ilist variables, int phase) {
    bdd r = phase ? bdd_true() : bdd_false();
    int i;
    for (i = 0; i < ilist_length(variables); i++) {
	bdd lit = bdd_ithvar(variables[i]);
	bdd nr = bdd_xor(r, lit);
	bdd_addref(nr);
	bdd_delref(r);
	r = nr;
    }
    bdd_delref(r);
    return r;
}
