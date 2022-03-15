#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "kernel.h"
#include "prover.h"
#include "tbdd.h"
#include "prime.h"

/*============================================
  Trust table declarations
============================================*/

/* Table entries */
typedef struct {
    BDD root;
    int clause_id;
    int refcount;
    int hash;
    int next;
} TrustNode;

/* Table parameters */
#define INIT_TABLE 1024
#define GROW_TABLE 1.4

/* Current table size */
static int table_count = 0;

/* Table */
static TrustNode *trust_table = NULL;

/* Free list head.  Chain through next links */
static int tfreepos = 0;
static int tfreenum = 0;

/* Accessing fields */
#define ROOT(t) (trust_table[t].root)
#define RNAME(t) (NNAME(ROOT(t)))
#define CLAUSE_ID(t) (trust_table[t].clause_id)

#define TMAXREF INT_MAX
#define TDECREF(t) if (trust_table[t].refcount!=TMAXREF && trust_table[t].refcount>0) trust_table[t].refcount--
#define TINCREF(t) if (trust_table[t].refcount<TMAXREF) trust_table[t].refcount++
#define THASREF(t) (trust_table[t].refcount>0)

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
  Trust table Operations
============================================*/

static void init_table() {
    int t;
    table_count = bdd_prime_gte(INIT_TABLE) + 2;
    trust_table = malloc(table_count * sizeof(TrustNode));
    if (!trust_table) {
	return bdd_error(BDD_MEMORY);
    }
    for (t = 0; t < table_count; t++) {
	trust_table[t].root = BDDZERO;
	trust_table[t].clause_id = 0;
	trust_table[t].refcount = 0;
	trust_table[t].hash = 0;
	trust_table[t].next = t+1;
    }
    trust_table[table_count-1].next = 0;
    /* TBDD Null */
    trust_table[0].root = BDDZERO;
    trust_table[0].clause_id = TAUTOLOGY;
    trust_table[0].refcount = TMAXREF;
    /* TBDD Tautology */
    trust_table[1].root = BDDONE;
    trust_table[1].clause_id = TAUTOLOGY;
    trust_table[1].refcount = TMAXREF;
    tfreepos = 2;
    tfreenum = table_count-2;
}

/* Make sure that nothing hashes to first two entries */
static int table_hash(BDD n) {
    return 2 + n % (table_count-2);
}

static void grow_table() {
    int t;
    int nsize = bdd_prime_gte((int) (GROW_TABLE * table_count)) + 2;
    trust_table = realloc(trust_table, nsize * sizeof(TrustNode));
    if (!trust_table)
	return bdd_error(BDD_MEMORY);
    for (t = table_count; t < nsize; t++) {
	trust_table[t].root = BDDZERO;
	trust_table[t].clause_id = 0;
	trust_table[t].refcount = 0;
    }
    /* Rehash entries */
    for (t = 0; t < table_count; t++)
	trust_table[t].hash = 0;
    table_count = nsize;
    tfreepos = 0;
    tfreenum = 0;
    for (t = table_count - 1; t >= 2; t--) {
	if (trust_table[t].clause_id == 0) {
	    trust_table[t].next = tfreepos;
	    tfreepos = t;
	    tfreenum++;
	} else {
	    int hash = table_hash(trust_table[t].root);
	    trust_table[t].next = trust_table[hash].hash;
	    trust_table[hash].hash = t;
	}
    }
}

/* Find entry in free list and remove */
static void free_tnode(TBDD t) {
    int hash = table_hash(ROOT(t));
    TBDD next = trust_table[hash];
    TBDD prev = 0;
    bool found = false;
    while (next != 0) {
	if (next ==  t) {
	    found = true;
	    if (prev == 0)
		trust_table[hash].hash = trust_table[t].next;
	    else {
		trust_table[prev].next = trust_table[t].next;
	    }
	    break;
	}
	prev = next;
	next = trust_table[next].next;
    }
    if (found) {
	trust_table[t].clause_id = 0;
	trust_table[t].next = tfreepos;
	tfreepos = t;
	tfreenum++;
    } else {
	fprintf(stderr, "WARNING: Didn't find trust node T%d in free list\n", t);
    }
}

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
    init_table();
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
    free(trust_table);
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


TBDD tbdd_make_node(BDD root, ilist antecedents) {
    if (root == BDDONE)
	return TBDD_TAUTOLOGY;

    int hash = table_hash(root);
    int res = trust_table[hash].hash;
    while (res !=  0) {
	if (ROOT(res) == root)
	    return tbdd_addref(res);
	res = trust_table[res].next;
    }
    {
	/* Build new trust node */
	int ubuf[1+ILIST_OVHD];
	ilist uclause = ilist_make(ubuf, 1);
	if (root != BDDZERO)
	    uclause = ilist_fill1(uclause, XVAR(root));
	if (tfreenum == 0)
	    grow_table();
	res = tfreepos;
	tfreepos = trust_table[tfreepos].next;
	if (root == BDDZERO)
	    print_proof_comment(2, "Trust node T%d: Empty clause\n", res);
	else
	    print_proof_comment(2, "Trust node T%d: Unit clause for node N%d\n", res, NNAME(root));
	int clause_id = generate_clause(uclause, antecedents);
	trust_table[res].root = bdd_addref(root);
	trust_table[res].clause_id = clause_id;
	trust_table[res].refcount = 1;
	trust_table[res].next = trust_table[hash].hash;
	trust_table[hash].hash = res;
    }
    return res;
}

/* 
   proof_step = TAUTOLOGY
   root = 1
 */
TBDD TBDD_tautology() {
    return TBDD_TAUTOLOGY;
}

/* 
   proof_step = TAUTOLOGY
   root = 0
 */
TBDD TBDD_null() {
    return TBDD_NULL;
}

bool tbdd_is_true(TBDD t) {
    return ROOT(t) == BDDONE;
}

bool tbdd_is_false(TBDD t) {
    return ROOT(t) == BDDZERO;
}

/*
  Increment/decrement reference count for BDD
 */
TBDD tbdd_addref(TBDD t) {
    TINCREF(t);
    return t;
}

void tbdd_delref(TBDD t) {
    if (!bddnodes)
	return;

    TDECREF(t);

    if (!THASREF(t)) {
	if (CLAUSE_ID(t) != TAUTOLOGY) {
	    int dbuf[1+ILIST_OVHD];
	    ilist dlist = ilist_make(dbuf, 1);
	    int id = CLAUSE_ID(t);
	    ilist_fill1(dlist, id);
	    BDD root = ROOT(t);
	    int nid = NNAME(root);
	    print_proof_comment(2, "Deleting Trust node T%d: Unit clause #%d for node N%d", t, id, nid);
	    delete_clauses(dlist);
	}
	bdd_delref(root);
	free_tnode(t);
    }
}


/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.
 */
static TBDD tbdd_from_clause_with_id(ilist clause, int id) {
    print_proof_comment(2, "Build BDD representation of clause #%d", id);
    clause = clean_clause(clause);
    BDD r = BDD_build_clause(clause);
    if (proof_type == PROOF_NONE) {
	return tbdd_makenode(r, TAUTOLOGY_CLAUSE);
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
    return tbdd_makenode(r, ant);
}

// This seems like it should be easier to check, but it isn't.
TBDD tbdd_from_clause(ilist clause) {
    int dbuf[ILIST_OVHD+1];
    ilist dels = ilist_make(dbuf, 1);
    int id = assert_clause(clause);
    TBDD t = tbdd_from_clause_with_id(clause, id);
    delete_clauses(ilist_fill1(dels, id));
    return t;
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
			    RNAME(result), ibuf, phase);
    }
    return result;
}


/*
  Upgrade ordinary BDD to TBDD by proving
  implication from another TBDD
 */
TBDD tbdd_validate(BDD r, TBDD t) {
    if (r == ROOT(t))
	return tbdd_addref(t);
    if (proof_type == PROOF_NONE) 
	return tbdd_makenode(r, TAUTOLOGY_CLAUSE);
    int abuf[2+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 2);
    pcbdd p = bdd_imptst_justify(ROOT(t), r);
    BDD root = p.root;
    if (root != bdd_true()) {
	fprintf(stderr, "Failed to prove implication N%d --> N%d\n", NNAME(root), NNAME(r));
	exit(1);
    }
    ilist_fill2(ant, p.clause_id, CLAUSE_ID(t));
    TBDD rt = tbdd_makenode(r, ant);
    /* Now we can handle any deletions caused by GC */
    process_deferred_deletions();
    return rt;
}

/*
  Declare BDD to be trustworthy.  Proof
  checker must provide validation.
  Only works when generating DRAT proofs
 */
TBDD tbdd_trust(BDD r) {
    if (proof_type == PROOF_NONE)
	return tbdd_makenode(r, TAUTOLOGY_CLAUSE);
    int abuf[0+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 0);
    return tbdd_makenode(r, ant);
}

/*
  Form conjunction of two TBDDs and prove
  their conjunction implies the new one
 */
TBDD tbdd_and(TBDD t1, TBDD t2) {
    if (proof_type == PROOF_NONE) {
	TBDD r = bdd_and(ROOT(t1), ROOT(t2));
	return tbdd_makenode(r, TAUTOLOGY_CLAUSE);
    }
    if (tbdd_is_true(t1))
	return tbdd_addref(t2);
    if (tbdd_is_true(t2))
	return tbdd_addref(t1);
    pcbdd p = bdd_and_justify(ROOT(t1), ROOT(t2));
    BDD r = p.root;
    int abuf[3+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 3);
    ilist_fill3(ant, CLAUSE_ID(t1), CLAUSE_ID(t2), p.clause_id);
    TBDD rt = tbdd_makenode(r, ant);
    process_deferred_deletions();
    return rt;
}

/*
  Form conjunction of TBDDs t1 & t2.  Use to validate
  BDD r
 */
TBDD tbdd_validate_with_and(BDD r, TBDD t1, TBDD t2) {
    if (proof_type == PROOF_NONE)
	return tbdd_trust(r);
    if (tbdd_is_true(t1))
	return tbdd_validate(r, t2);
    if (tbdd_is_true(t2))
	return tbdd_validate(r, t1);
    pcbdd p = bdd_and_imptst_justify(ROOT(t1), ROOT(t2), r);
    if (p.root != bdd_true()) {
	fprintf(stderr, "Failed to prove implication N%d & N%d --> N%d\n", RNAME(t1), RNAME(t2), NNAME(r));
	exit(1);
    }
    int abuf[3+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 3);
    ilist_fill3(ant, CLAUSE_ID(t1), CLAUSE_ID(t2), p.clause_id);
    TBDD rt = tbdd_makenode(r, ant);
    /* Now we can handle any deletions caused by GC */
    process_deferred_deletions();
    return rt;
}

/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */

/* See if can validate clause directly from path in BDD */

static bool test_validation_path(ilist clause, TBDD t) {
    // Put in descending order by level
    clause = clean_clause(clause);
    int len = ilist_length(clause);
    int i;
    BDD r = ROOT(t);
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

static int tbdd_validate_clause_path(ilist clause, TBDD t) {
    int len = ilist_length(clause);
    int abuf[1+len+ILIST_OVHD];
    int i;
    BDD r = ROOT(t);
    ilist ant = ilist_make(abuf, 1+len);
    ilist_fill1(ant, CLAUSE_ID(t));
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
	print_proof_comment(2, "Validation of clause [%s] from N%d", buf, RNAME(t));
    }
    int id =  generate_clause(clause, ant);
    return id;
}

int tbdd_validate_clause(ilist clause, TBDD t) {
    if (proof_type == PROOF_NONE)
	return TAUTOLOGY;
    clause = clean_clause(clause);
    if (test_validation_path(clause, t)) {
	return tbdd_validate_clause_path(clause, t);
    } else {
	if (verbosity_level >= 2) {
	    char buf[BUFLEN];
	    ilist_format(clause, buf, " ", BUFLEN);
	    print_proof_comment(2, "Validation of clause [%s] from N%d requires generating intermediate BDD", buf, RNAME(t));
	}
	BDD cr = BDD_build_clause(clause);
	bdd_addref(cr);
	TBDD tc = tbdd_validate(cr, t);
	bdd_delref(cr);
	int id = tbdd_validate_clause_path(clause, tc);
	if (id < 0) {
	    char buf[BUFLEN];
	    ilist_format(clause, buf, " ", BUFLEN);
	    print_proof_comment(2, "Oops.  Couldn't validate clause [%s] from N%d", buf, RNAME(t));
	}
	tbdd_delref(tc);
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
