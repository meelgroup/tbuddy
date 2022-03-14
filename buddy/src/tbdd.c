#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "prover.h"
#include "tbdd.h"


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

int tbdd_init(FILE *pfile, int *variable_counter, int *clause_id_counter, ilist *input_clauses, ilist variable_ordering, proof_type_t ptype, bool binary) {
    return prover_init(pfile, variable_counter, clause_id_counter, input_clauses, variable_ordering, ptype, binary);
}

int tbdd_init_lrat(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses, ilist variable_ordering) {
    last_variable = variable_count;
    last_clause_id = clause_count;
    return tbdd_init(pfile, &last_variable, &last_clause_id, input_clauses, variable_ordering, PROOF_LRAT, false);
}

int tbdd_init_lrat_binary(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses, ilist variable_ordering) {
    last_variable = variable_count;
    last_clause_id = clause_count;
    return tbdd_init(pfile, &last_variable, &last_clause_id, input_clauses, variable_ordering, PROOF_LRAT, true);
}

int tbdd_init_drat(FILE *pfile, int variable_count) {
    last_variable = variable_count;
    last_clause_id = 0;
    return tbdd_init(pfile, &last_variable, &last_clause_id, NULL, NULL, PROOF_DRAT, false);
}

int tbdd_init_drat_binary(FILE *pfile, int variable_count) {
    last_variable = variable_count;
    last_clause_id = 0;
    return tbdd_init(pfile, &last_variable, &last_clause_id, NULL, NULL, PROOF_DRAT, true);
}

int tbdd_init_frat(FILE *pfile, int *variable_counter, int *clause_id_counter) {
    return tbdd_init(pfile, variable_counter, clause_id_counter, NULL, NULL, PROOF_FRAT, false);
}

int tbdd_init_frat_binary(FILE *pfile, int *variable_counter, int *clause_id_counter) {
    return tbdd_init(pfile, variable_counter, clause_id_counter, NULL, NULL, PROOF_FRAT, false);
}

int tbdd_init_noproof(int variable_count) {
    last_variable = variable_count;
    return prover_init(NULL, &last_variable, NULL, NULL, NULL, PROOF_NONE, false);
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
	printf("\nc BDD statistics\n");
	printf("c ----------------\n");
	printf("c Total BDD nodes produced: %ld\n", s.produced);
    }
    bdd_done();
    if (verbosity_level >= 1) {
	printf("c Input variables: %d\n", input_variable_count);
	printf("c Input clauses: %d\n", input_clause_count);
	printf("c Total clauses: %d\n", total_clause_count);
	printf("c Maximum live clauses: %d\n", max_live_clause_count);
	printf("c Deleted clauses: %d\n", deleted_clause_count);
	printf("c Final live clauses: %d\n", total_clause_count-deleted_clause_count);
	if (variable_counter)
	    printf("c Total variables: %d\n", *variable_counter);
    }
    for (i = 0; i < ifun_count; i++) {
	ifuns[i](verbosity_level);
    }
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
  Make copy of TBDD
 */
static TBDD tbdd_duplicate(TBDD tr) {
    TBDD rr;
    rr.root = bdd_addref(tr.root);
    rr.clause_id = tr.clause_id;
    return rr;
}

/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.
 */


static TBDD tbdd_from_clause_with_id(ilist clause, int id) {
    TBDD rr;
    print_proof_comment(2, "Build BDD representation of clause #%d", id);
    clause = clean_clause(clause);
    BDD r = bdd_addref(BDD_build_clause(clause));
    if (proof_type == PROOF_NONE) {
	rr.clause_id = TAUTOLOGY;
	return rr;
    }
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
    rr.root = r;
    rr.clause_id = generate_clause(uclause, ant);
    print_proof_comment(2, "Validate BDD representation of Clause #%d.  Node = N%d.", id, NNAME(rr.root));
    return rr;
}

TBDD tbdd_from_clause_old(ilist clause) {
    if (verbosity_level >= 2) {
	ilist_format(clause, ibuf, " ", BUFLEN);
	print_proof_comment(2, "BDD representation of clause [%s]", ibuf);
    }
    BDD r = BDD_build_clause(clause);
    return tbdd_trust(r);    
}

// This seems like it should be easier to check, but it isn't.
TBDD tbdd_from_clause(ilist clause) {
    int dbuf[ILIST_OVHD+1];
    ilist dels = ilist_make(dbuf, 1);
    int id = assert_clause(clause);
    TBDD tr = tbdd_from_clause_with_id(clause, id);
    delete_clauses(ilist_fill1(dels, id));
    return tr;
}


TBDD tbdd_from_clause_id(int id) {
    ilist clause = get_input_clause(id);
    if (clause == NULL) {
	fprintf(stderr, "Invalid input clause #%d\n", id);
	exit(1);
    }
    return tbdd_from_clause_with_id(clause, id);
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
    ilist_sort(vars);
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
	TBDD tc = tbdd_from_clause(lits);
	if (tbdd_is_true(result)) {
	    result = tc;
	} else {
	    TBDD nresult = tbdd_and(result, tc);
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
    TBDD rr;
    if (r == tr.root)
	return tbdd_duplicate(tr);
    if (proof_type == PROOF_NONE) {
	rr.root = bdd_addref(r);
	rr.clause_id = TAUTOLOGY;
	return rr;
    }
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[2+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 2);
    pcbdd t = bdd_imptst_justify(tr.root, bdd_addref(r));
    if (t.root != bdd_true()) {
	fprintf(stderr, "Failed to prove implication N%d --> N%d\n", NNAME(tr.root), NNAME(r));
	exit(1);
    }
    print_proof_comment(2, "Validation of unit clause for N%d by implication from N%d",NNAME(r), NNAME(tr.root));
    ilist_fill1(clause, XVAR(r));
    ilist_fill2(ant, t.clause_id, tr.clause_id);
    rr.clause_id = generate_clause(clause, ant);
    rr.root = r;
    /* Now we can handle any deletions caused by GC */
    process_deferred_deletions();
    return rr;
}

/*
  Declare BDD to be trustworthy.  Proof
  checker must provide validation.
  Only works when generating DRAT proofs
 */
TBDD tbdd_trust(BDD r) {
    TBDD rr;
    if (proof_type == PROOF_NONE) {
	rr.root = bdd_addref(r);
	rr.clause_id = TAUTOLOGY;
	return rr;
    }
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[0+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 0);
    print_proof_comment(2, "Assertion of N%d",NNAME(r));
    ilist_fill1(clause, XVAR(r));
    rr.clause_id = generate_clause(clause, ant);
    rr.root = bdd_addref(r);
    return rr;
}

/*
  Form conjunction of two TBDDs and prove
  their conjunction implies the new one
 */
TBDD tbdd_and(TBDD tr1, TBDD tr2) {
    if (proof_type == PROOF_NONE) {
	TBDD rr;
	rr.root = bdd_addref(bdd_and(tr1.root, tr2.root));
	rr.clause_id = TAUTOLOGY;
	return rr;
    }
    if (tbdd_is_true(tr1))
	return tbdd_duplicate(tr2);
    if (tbdd_is_true(tr2))
	return tbdd_duplicate(tr1);
    pcbdd t = bdd_and_justify(tr1.root, tr2.root);
    TBDD rr;
    rr.root = bdd_addref(t.root);
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[3+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 3);
    print_proof_comment(2, "Validate unit clause for node N%d = N%d & N%d", NNAME(t.root), NNAME(tr1.root), NNAME(tr2.root));
    ilist_fill1(clause, XVAR(t.root));
    ilist_fill3(ant, tr1.clause_id, tr2.clause_id, t.clause_id);
    /* Insert proof of unit clause into t's justification */
    rr.clause_id = generate_clause(clause, ant);
    /* Now we can handle any deletions caused by GC */
    process_deferred_deletions();
    return rr;
}

#define OLD 0

/*
  Form conjunction of TBDDs tr1 & tr2.  Use to validate
  BDD r
 */
TBDD tbdd_validate_with_and(BDD r, TBDD tr1, TBDD tr2) {
    TBDD rr;
#if OLD
    TBDD ta = tbdd_and(tr1, tr2);
    rr = tbdd_validate(r, ta);
    return tres;
#else
    if (proof_type == PROOF_NONE)
	return tbdd_trust(r);
    if (tbdd_is_true(tr1))
	return tbdd_validate(r, tr2);
    if (tbdd_is_true(tr2))
	return tbdd_validate(r, tr1);
    pcbdd t = bdd_and_imptst_justify(tr1.root, tr2.root, bdd_addref(r));
    if (t.root != bdd_true()) {
	fprintf(stderr, "Failed to prove implication N%d & N%d --> N%d\n", NNAME(tr1.root), NNAME(tr2.root), NNAME(r));
	exit(1);
    }
    int cbuf[1+ILIST_OVHD];
    ilist clause = ilist_make(cbuf, 1);
    int abuf[3+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 3);
    print_proof_comment(2, "Validate unit clause for node N%d, based on N%d & N%d", NNAME(r), NNAME(tr1.root), NNAME(tr2.root));
    ilist_fill1(clause, XVAR(r));
    ilist_fill3(ant, tr1.clause_id, tr2.clause_id, t.clause_id);
    /* Insert proof of unit clause into rr's justification */
    rr.root = r;
    rr.clause_id = generate_clause(clause, ant);
    /* Now we can handle any deletions caused by GC */
    process_deferred_deletions();
#endif
    return rr;
}

/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */

/* See if can validate clause directly from path in BDD */

static bool test_validation_path(ilist clause, TBDD tr) {
    // Put in descending order by level
    clause = clean_clause(clause);
    int len = ilist_length(clause);
    int i;
    BDD r = tr.root;
    for (i = len-1; i >= 0; i--) {
	int lit = clause[i];
	int var = ABS(lit);
	int level = bdd_var2level(var);
	if (LEVEL(r) > level)
	    // Function does not depend on this variable
	    continue;
	if (LEVEL(r) < level)
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
    if (proof_type == PROOF_NONE)
	return TAUTOLOGY;
    clause = clean_clause(clause);
    if (test_validation_path(clause, tr)) {
	return tbdd_validate_clause_path(clause, tr);
    } else {
	if (verbosity_level >= 2) {
	    char buf[BUFLEN];
	    ilist_format(clause, buf, " ", BUFLEN);
	    print_proof_comment(2, "Validation of clause [%s] from N%d requires generating intermediate BDD", buf, NNAME(tr.root));
	}
	BDD cr = BDD_build_clause(clause);
	bdd_addref(cr);
	TBDD tcr = tbdd_validate(cr, tr);
	bdd_delref(cr);
	int id = tbdd_validate_clause_path(clause, tcr);
	if (id < 0) {
	    char buf[BUFLEN];
	    ilist_format(clause, buf, " ", BUFLEN);
	    print_proof_comment(2, "Oops.  Couldn't validate clause [%s] from N%d", buf, NNAME(tr.root));
	}
	tbdd_delref(tcr);
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
    if (proof_type == PROOF_NONE)
	return TAUTOLOGY;
    int abuf[1+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 1);
    if (verbosity_level >= 2) {
	char buf[BUFLEN];
	ilist_format(clause, buf, " ", BUFLEN);
	print_proof_comment(2, "Assertion of clause [%s]", buf);
    }
    return generate_clause(clause, ant);
}

/*============================================
 Useful BDD operations
============================================*/

/*
  Build BDD representation of XOR (phase = 1) or XNOR (phase = 0)
*/
BDD BDD_build_xor(ilist vars, int phase) {
    if (ilist_length(vars) == 0)
	return phase ? bdd_false() : bdd_true();
    ilist variables = ilist_copy(vars);
    /* Put into descending order by level */
    variables = clean_clause(variables);
    int n = ilist_length(variables);
    BDD even = bdd_addref(bdd_true());
    BDD odd = bdd_addref(bdd_false());
    int i, var, level;
    //    printf("Building Xor for [");
    //    ilist_print(variables, stdout, " ");
    //    printf("]\n");

    for (i = 0; i < n-1; i++) {
	var = variables[i];
	level = bdd_var2level(var);
	BDD neven = bdd_addref(bdd_makenode(level, even, odd));
	BDD nodd = bdd_addref(bdd_makenode(level, odd, even));
	bdd_delref(even);
	bdd_delref(odd);
	even = neven;
	odd = nodd;
    }
    var = variables[n-1];
    level = bdd_var2level(var);
    BDD r = phase ? bdd_makenode(level, odd, even) : bdd_makenode(level, even, odd);
    bdd_delref(even);
    bdd_delref(odd);
    ilist_free(variables);
    return r;
}

/*
  Build BDD representation of clause.
 */
BDD BDD_build_clause(ilist literals) {
    /* Put into descending order by level */
    literals = clean_clause(literals);
    if (literals == TAUTOLOGY_CLAUSE)
	return bdd_true();
    //    printf("Building BDD for clause [");
    //    ilist_print(literals, stdout, " ");
    //    printf("]\n");
    BDD r = bdd_false();
    int i, lit, var, level;
    for (i = 0; i < ilist_length(literals); i++) {
	bdd_addref(r);
	lit = literals[i];
	var = ABS(lit);
	level = bdd_var2level(var);
	BDD nr = lit < 0 ? bdd_makenode(level, bdd_true(), r) : bdd_makenode(level, r, bdd_true());
	bdd_delref(r);
	r = nr;
    }
    return r;
}

/*
  Build BDD representation of conjunction of literals (a "cube")
 */
BDD BDD_build_cube(ilist literals) {
    if (literals == FALSE_CUBE)
	return bdd_false();
    /* Put into descending order by level */
    literals = clean_clause(literals);
    BDD r = bdd_true();
    int i, lit, var, level;
    for (i = 0; i < ilist_length(literals); i++) {
	bdd_addref(r);
	lit = literals[i];
	var = ABS(lit);
	level = bdd_var2level(var);
	BDD nr = lit < 0 ? bdd_makenode(level, r, bdd_false()) : bdd_makenode(level, bdd_false(), r);
	bdd_delref(r);
	r = nr;
    }
    return r;
}

ilist BDD_decode_cube(BDD r) {
    ilist literals = ilist_new(1);
    if (r == bdd_false())
	return FALSE_CUBE;
    while (r != bdd_true()) {
	int var = bdd_var(r);
	if (bdd_high(r) == bdd_false()) {
	    literals = ilist_push(literals, -var);
	    r = bdd_low(r);
	} else {
	    literals = ilist_push(literals, var);
	    r = bdd_high(r);
	}
    }
    return literals;

}
