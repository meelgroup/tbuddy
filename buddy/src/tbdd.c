#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tbdd.h"
#include "kernel.h"

/*============================================
  Local data
============================================*/

#define BUFLEN 2048
// For formatting information
static char ibuf[BUFLEN];

#define FUN_MAX 10
static tbdd_info_fun ifuns[FUN_MAX];
static int ifun_count = 0;

static tbdd_done_fun dfuns[FUN_MAX];
static int dfun_count = 0;

static int last_variable = 0;
static int last_clause_id = 0;


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
  
  When generating DRAT proofs, can provide NULL for argument input_clauses.

  These functions also initialize BuDDy, using parameters tuned according
  to the predicted complexity of the operations.

  Returns 0 if OK, otherwise error code
*/

int tbdd_init(FILE *pfile, int *variable_counter, int *clause_id_counter, ilist *input_clauses, proof_type_t ptype, bool binary) {
    return prover_init(pfile, variable_counter, clause_id_counter, input_clauses, ptype, binary);
}

int tbdd_init_lrat(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses) {
    last_variable = variable_count;
    last_clause_id = clause_count;
    return tbdd_init(pfile, &last_variable, &last_clause_id, input_clauses, PROOF_LRAT, false);
}

int tbdd_init_lrat_binary(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses) {
    last_variable = variable_count;
    last_clause_id = clause_count;
    return tbdd_init(pfile, &last_variable, &last_clause_id, input_clauses, PROOF_LRAT, true);
}

int tbdd_init_drat(FILE *pfile, int variable_count) {
    last_variable = variable_count;
    last_clause_id = 0;
    return tbdd_init(pfile, &last_variable, &last_clause_id, NULL, PROOF_DRAT, false);
}

int tbdd_init_drat_binary(FILE *pfile, int variable_count) {
    last_variable = variable_count;
    last_clause_id = 0;
    return tbdd_init(pfile, &last_variable, &last_clause_id, NULL, PROOF_DRAT, true);
}

int tbdd_init_frat(FILE *pfile, int *variable_counter, int *clause_id_counter) {
    return tbdd_init(pfile, variable_counter, clause_id_counter, NULL, PROOF_FRAT, false);
}

int tbdd_init_frat_binary(FILE *pfile, int *variable_counter, int *clause_id_counter) {
    return tbdd_init(pfile, variable_counter, clause_id_counter, NULL, PROOF_FRAT, false);
}

void tbdd_set_verbose(int level) {
    verbosity_level = level;
}

void tbdd_done() {
    int i;
    for (i = 0; i < dfun_count; i++)
	dfuns[i]();
    prover_done();
    if (verbosity_level >= 1) {
	bddStat s;
	bdd_stats(&s);
	bdd_printstat();
	printf("Total BDD nodes: %ld\n", s.produced);
	printf("Total clauses: %d\n", s.clausenum);
	printf("Max live clauses: %d\n", s.maxclausenum);
	printf("Total variables: %d\n", s.variablenum);
    }
    for (i = 0; i < ifun_count; i++) {
	ifuns[i](verbosity_level);
    }
    bdd_done();
}

void tbdd_add_info_fun(tbdd_info_fun f) {
    if (ifun_count >= FUN_MAX) {
	fprintf(stderr, "Limit of %d TBDD information functions.  Request ignored\n", FUN_MAX);
	return;
    }
    ifuns[ifun_count++] = f;
}

void tbdd_add_done_fun(tbdd_done_fun f) {
    if (dfun_count >= FUN_MAX) {
	fprintf(stderr, "Limit of %d TBDD done functions.  Request ignored\n", FUN_MAX);
	return;
    }
    dfuns[dfun_count++] = f;
}


/* 
   proof_step = TAUTOLOGY
   root = 1
 */
TBDD TBDD_tautology() {
    TBDD rr;
    rr.root = bdd_true();
    rr.clause_id = TAUTOLOGY;
    return rr;
}

/* 
   proof_step = TAUTOLOGY
   root = 0
 */
TBDD TBDD_null() {
    TBDD rr;
    rr.root = bdd_false();
    rr.clause_id = TAUTOLOGY;
    return rr;
}

bool tbdd_is_true(TBDD tr) {
    return ISONE(tr.root);
}

bool tbdd_is_false(TBDD tr) {
    return ISZERO(tr.root);
}

/*
  Increment/decrement reference count for BDD
 */
TBDD tbdd_addref(TBDD tr) {
    bdd_addref(tr.root); return tr;
}

void tbdd_delref(TBDD tr) {
    if (!bddnodes)
	return;
    bdd_delref(tr.root);

    if (!HASREF(tr.root) && tr.clause_id != TAUTOLOGY) {
	int dbuf[1+ILIST_OVHD];
	ilist dlist = ilist_make(dbuf, 1);
	ilist_fill1(dlist, tr.clause_id);
	print_proof_comment(2, "Deleting unit clause #%d for node N%d", tr.clause_id, NNAME(tr.root));
	delete_clauses(dlist);
    }
    tr.clause_id = TAUTOLOGY;
}

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
	BDD vr = bdd_addref(lit < 0 ? BDD_nithvar(-lit) : BDD_ithvar(lit));
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
    if (verbosity_level >= 2) {
	ilist_format(clause, ibuf, " ", BUFLEN);
	print_proof_comment(2, "BDD representation of clause [%s]", ibuf);
    }
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
    rr.clause_id = generate_clause(uclause, ant);
    print_proof_comment(2, "Validate BDD representation of Clause #%d.  Node = N%d.", id, NNAME(rr.root));
    return rr;
}

/*
  For generating xor's: does word have odd or even parity?
*/
static int parity(int w) {
  int odd = 0;
  while (w > 0) {
    odd ^= w & 0x1;
    w >>= 1;
  }
  return odd;
}

/*
  Sort integers in ascending order
 */
int int_compare_tbdd(const void *i1p, const void *i2p) {
  int i1 = *(int *) i1p;
  int i2 = *(int *) i2p;
  if (i1 < i2)
    return -1;
  if (i1 > i2)
    return 1;
  return 0;
}

/*
  Generate BDD for arbitrary-input xor, using
  clauses to ensure that DRAT can validate result
*/

TBDD TBDD_from_xor(ilist vars, int phase) {
    qsort((void *) vars, ilist_length(vars), sizeof(int), int_compare_tbdd);
    int len = ilist_length(vars);
    int bits;
    int elen = 1 << len;
    int lbuf[ILIST_OVHD+len];
    ilist lits = ilist_make(lbuf, len);
    ilist_resize(lits, len);
    TBDD result = TBDD_tautology();
    for (bits = 0; bits < elen; bits++) {
	int i;
	if (parity(bits) == phase)
	    continue;
	for (i = 0; i < len; i++)
	    lits[i] = (bits >> i) & 0x1 ? -vars[i] : vars[i];
	TBDD tc = tbdd_addref(tbdd_from_clause(lits));
	if (tbdd_is_true(result)) {
	    result = tc;
	} else {
	    TBDD nresult = tbdd_addref(tbdd_and(result, tc));
	    tbdd_delref(tc);
	    tbdd_delref(result);
	    result = nresult;
	}
    }
    if (verbosity_level >= 2) {
	ilist_format(vars, ibuf, " ^ ", BUFLEN);
	print_proof_comment(2, "N%d is BDD representation of %s = %d",
			    bdd_nameid(result.root), ibuf, phase);
    }
    return result;
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
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[0+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 0);
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
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[3+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 3);
    print_proof_comment(2, "Validate unit clause for node N%d = N%d & N%d", NNAME(t.root), NNAME(tr1.root), NNAME(tr2.root));
    ilist_fill1(clause, XVAR(t.root));
    ilist_fill3(ant, tr1.clause_id, tr2.clause_id, t.clause_id);
    /* Insert proof of unit clause into t's justification */
    t.clause_id = generate_clause(clause, ant);
    bdd_delref(tr1.root);
    bdd_delref(tr2.root);
    return t;
}

/*
  Form conjunction of TBDDs tl & tr.  Use to validate
  BDD r
 */
TBDD tbdd_validate_with_and(BDD r, TBDD tl, TBDD tr) {
    TBDD ta = tbdd_and(tl, tr);
    bdd_addref(ta.root);
    TBDD tres = tbdd_validate(r, ta);
    bdd_delref(ta.root);
    return tres;
}


/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */

/* See if can validate clause directly from path in BDD */

static bool test_validation_path(ilist clause, TBDD tr) {
    int len = ilist_length(clause);
    int i;
    BDD r = tr.root;
    for (i = len-1; i >= 0; i--) {
	int lit = clause[i];
	int var = ABS(lit);
	if (LEVEL(r) > var)
	    // Function does not depend on this variable
	    continue;
	if (LEVEL(r) < var)
	    // Cannot validate clause directly
	    return false;
	if (lit < 0)
	    r = HIGH(r);
	else
	    r = LOW(r);
    }
    return ISZERO(r);
}

static int tbdd_validate_clause_path(ilist clause, TBDD tr) {
    int len = ilist_length(clause);
    int abuf[1+len+ILIST_OVHD];
    int i;
    BDD r = tr.root;
    ilist ant = ilist_make(abuf, 1+len);
    ilist_fill1(ant, tr.clause_id);
    for (i = len-1; i >= 0; i--) {
	int lit = clause[i];
	int var = ABS(lit);
	int id;
	if (LEVEL(r) > var)
	    // Function does not depend on this variable
	    continue;
	if (LEVEL(r) < var)
	    // Cannot validate clause directly
	    return -1;
	if (lit < 0) {
	    id = bdd_dclause(r, DEF_HD);
	    r = HIGH(r);
	} else {
	    id = bdd_dclause(r, DEF_LD);
	    r = LOW(r);
	}
	if (id != TAUTOLOGY)
	    ilist_push(ant, id);
    }
    if (verbosity_level >= 2) {
	char buf[BUFLEN];
	ilist_format(clause, buf, " ", BUFLEN);
	print_proof_comment(2, "Validation of clause [%s] from N%d", buf, NNAME(tr.root));
    }
    int id =  generate_clause(clause, ant);
    return id;
}



int tbdd_validate_clause(ilist clause, TBDD tr) {
    clause = clean_clause(clause);
    if (test_validation_path(clause, tr)) {
	return tbdd_validate_clause_path(clause, tr);
    } else {
	if (verbosity_level >= 2) {
	    char buf[BUFLEN];
	    ilist_format(clause, buf, " ", BUFLEN);
	    print_proof_comment(2, "Validation of clause [%s] from N%d requires generating intermediate BDD", buf, NNAME(tr.root));
	}
	BDD cr = bdd_addref(bdd_from_clause(clause));
	TBDD tcr = tbdd_addref(tbdd_validate(cr, tr));
	int id = tbdd_validate_clause_path(clause, tcr);
	if (id < 0) {
	    char buf[BUFLEN];
	    ilist_format(clause, buf, " ", BUFLEN);
	    print_proof_comment(2, "Oops.  Couldn't validate clause [%s] from N%d", buf, NNAME(tr.root));
	}
	return id;
    }
}

/*
  Assert that a clause holds.  Proof checker
  must provide validation.
  Use this version when generating DRAT proofs,
  or when don't want to provide antecedent in FRAT proof
  Returns clause id.
 */
int assert_clause(ilist clause) {
    int abuf[1+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 1);
    if (verbosity_level >= 2) {
	char buf[BUFLEN];
	ilist_format(clause, buf, " ", BUFLEN);
	print_proof_comment(2, "Assertion of clause [%s]", buf);
    }
    return generate_clause(clause, ant);
}

