#include <stdio.h>
#include <stdlib.h>
#include "prover.h"
#include "kernel.h"


/* Global variables exported by prover */
bool do_lrat = false;
int verbosity_level = 1;
int last_variable = 0;
int last_clause_id = 0;
int total_clause_count = 0;
int input_variable_count = 0;
int max_live_clause_count = 0;

/* Global variables used by prover */
static FILE *proof_file = NULL;
/* 
   For LRAT, only need to keep dictionary of input clauses.
   For DRAT, need dictionary of all clauses in order to delete them.
*/

static ilist *all_clauses = NULL;
static int input_clause_count = 0;
static int alloc_clause_count = 0;
static int live_clause_count = 0;

// Parameters
// Cutoff betweeen large and small allocations (in terms of clauses)
#define BUDDY_THRESHOLD 1000
//#define BUDDY_THRESHOLD 10
#define BUDDY_NODES_LARGE (1*1000*1000)
//#define BUDDY_NODES_LARGE (1000)
#define BUDDY_NODES_SMALL (    200*1000)
#define BUDDY_CACHE_RATIO 8
#define BUDDY_INCREASE_RATIO 20

// How many clauses should allocated for clauses
#define INITIAL_CLAUSE_COUNT 1000


/* Useful static functions */


/* API functions */

int prover_init(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses, bool lrat) {
    do_lrat = lrat;
    proof_file = pfile;
    print_proof_comment(0, "Proof of CNF file with %d variables and %d clauses", variable_count, clause_count);    

    input_variable_count = last_variable = variable_count;
    last_clause_id = input_clause_count = total_clause_count = clause_count;
    live_clause_count = max_live_clause_count = clause_count;
    alloc_clause_count = clause_count + INITIAL_CLAUSE_COUNT;
    all_clauses = calloc(alloc_clause_count, sizeof(ilist *));
    if (all_clauses == NULL) {
	return bdd_error(BDD_MEMORY);
    }
    int cid;
    for (cid = 0; cid < clause_count; cid++) {
	all_clauses[cid] = ilist_copy(input_clauses[cid]);
	print_proof_comment(2, "Input Clause #%d.  %d literals", cid+1, ilist_length(all_clauses[cid]));
    }
    

    int bnodes = clause_count < BUDDY_THRESHOLD ? BUDDY_NODES_SMALL : BUDDY_NODES_LARGE;
    int bcache = bnodes/BUDDY_CACHE_RATIO;
    int bincrease = bnodes/BUDDY_INCREASE_RATIO;
    int rval = bdd_init(bnodes, bcache);

    bdd_setcacheratio(BUDDY_CACHE_RATIO);
    bdd_setmaxincrease(bincrease);
    bdd_setvarnum(variable_count+1);

    return rval;
}

void prover_done() {
    bdd_done();
}

/* Print clause */
void print_clause(FILE *out, ilist clause) {
    if (clause == TAUTOLOGY_CLAUSE) {
	fprintf(out, "TAUT");
	return;
    }
    char *bstring = "[";
    int i;
    for (i = 0; i < ilist_length(clause); i++) {
	int lit = clause[i];
	if (lit == TAUTOLOGY) 
	    fprintf(out, "%sTRUE", bstring);
	else if (lit == -TAUTOLOGY)
	    fprintf(out, "%sFALSE", bstring);
	else
	    fprintf(out, "%s%d", bstring, lit);
	bstring = ", ";
    }
    fprintf(out, "]");
}

/* Helper function for clause cleaning.  Sort literals to put variables in descending order */
int literal_compare(const void *l1p, const void *l2p) {
     int l1 = *(int *) l1p;
     int l2 = *(int *) l2p;
     unsigned u1 = l1 < 0 ? -l1 : l1;
     unsigned u2 = l2 < 0 ? -l2 : l2;
     if (u2 > u1)
	 return 1;
     if (u2 < u1)
	 return -1;
     return 0;
}

static ilist clean_clause(ilist clause) {
#if 0
    printf("Cleaning clause : [");
    ilist_print(clause, stdout, " ");
    printf("]\n");
#endif
    if (clause == TAUTOLOGY_CLAUSE)
	return clause;
    int len = ilist_length(clause);
    if (len == 0)
	return clause;
    /* Sort the literals */
    qsort((void *) clause, ilist_length(clause), sizeof(int), literal_compare);
    int geti = 0;
    int puti = 0;
    int plit = 0;
    while (geti < len) {
	int lit = clause[geti++];
	if (lit == TAUTOLOGY)
	    return TAUTOLOGY_CLAUSE;
	if (lit == -TAUTOLOGY)
	    continue;
	if (lit == plit)
	    continue;
	if (lit == -plit)
	    return TAUTOLOGY_CLAUSE;
	clause[puti++] = lit;
	plit = lit;
    }
    clause = ilist_resize(clause, puti);
#if 0
    printf("Result clause : [");
    ilist_print(clause, stdout, " ");
    printf("]\n");
#endif
    return clause;
}

static ilist clean_hints(ilist hints) {
    int len = ilist_length(hints);
    int geti = 0;
    int puti = 0;
    while (geti < len) {
	int lit = hints[geti++];
	if (lit != TAUTOLOGY)
	    hints[puti++] = lit;
    }
    hints = ilist_resize(hints, puti);
    return hints;
}


/* Return clause ID */
/* For DRAT proof, antecedents can be NULL */
int generate_clause(ilist literals, ilist hints) {
    ilist clause = clean_clause(literals);
    int cid = ++last_clause_id;
    hints = clean_hints(hints);
    if (clause == TAUTOLOGY_CLAUSE)
	return TAUTOLOGY;
    if (do_lrat)
	fprintf(proof_file, "%d ", cid);
    ilist_print(clause, proof_file, " ");
    if (do_lrat) {
	fprintf(proof_file, " 0 ");
	ilist_print(hints, proof_file, " ");
    }
    fprintf(proof_file, " 0\n");
    total_clause_count++;
    live_clause_count++;
    max_live_clause_count = MAX(max_live_clause_count, live_clause_count);
    
    if (!do_lrat) {
	/* Must store copy of clause */
	if (alloc_clause_count <= cid) {
	    /* must expand */
	    alloc_clause_count *= 2;
	    all_clauses = realloc(all_clauses, alloc_clause_count * sizeof(int));
	    if (all_clauses == NULL)
		return bdd_error(BDD_MEMORY);
	}
	all_clauses[cid-1] = ilist_copy(clause);
    }
    return cid;
}


void delete_clauses(ilist clause_ids) {
    if (do_lrat) {
	fprintf(proof_file, "%d d ", last_clause_id);
	ilist_print(clause_ids, proof_file, " ");
	fprintf(proof_file, " 0\n");
    } else {
	int cid;
	for (cid = 0; cid < ilist_length(clause_ids); cid++) {
	    fprintf(proof_file, "d ");
	    ilist_print(all_clauses[cid], proof_file, " ");
	    fprintf(proof_file, " 0\n");
	    ilist_free(all_clauses[cid]);
	    all_clauses[cid] = NULL;
	}
    }
    live_clause_count -= ilist_length(clause_ids);
}

/* Retrieve input clause.  NULL if invalid */
ilist get_input_clause(int id) {
    if (id > input_clause_count)
	return NULL;
    return all_clauses[id-1];
}


void print_proof_comment(int vlevel, char *fmt, ...) {
    if (proof_file == NULL) {
	fprintf(stderr, "When printing comment.  Invalid proof file.  Switching to stdout\n");
	proof_file = stdout;
    }
    if (verbosity_level >= vlevel) {
	va_list vlist;
	fprintf(proof_file, "c ");
	va_start(vlist, fmt);
	vfprintf(proof_file, fmt, vlist);
	va_end(vlist);
	fprintf(proof_file, "\n");
    }
}

/* 
   Fill ilist with defining clause.
   ils should be big enough for 3 elemeents.
   Result should be processed with clean_clause()
*/
ilist defining_clause(ilist ils, dclause_t dtype, int nid, int vid, int hid, int lid) {
    switch(dtype) {
    case DEF_HU:
	ils = ilist_fill3(ils,  nid, -vid, -hid);
	break;
    case DEF_LU:
	ils = ilist_fill3(ils,  nid,  vid, -lid);
	break;
    case DEF_HD:
	ils = ilist_fill3(ils, -nid, -vid,  hid);
	break;
    case DEF_LD:
	ils = ilist_fill3(ils, -nid,  vid,  lid);
	break;
    default:
	/* Should throw exception here */
	ils = TAUTOLOGY_CLAUSE;
    }
    return ils;
}

/******* Support for Apply Proof generation *****/

/*
  Maxima
 */
#define MAX_CLAUSE 4
#define MAX_HINT 8

/*
  Enumerated type for the hint types
 */
typedef enum { HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH, HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL, HINT_EXTRA } jtype_t;

#define HINT_COUNT HINT_EXTRA

/* There are 8 basic hints, plus possibly an extra one from an intermediate clause */
const char *hint_name[HINT_COUNT+1] = {"RESHU", "ARG1HD", "ARG2HD", "OPH", "RESLU", "ARG1LD", "ARG2LD", "OPL", "EXTRA"};

/*
  Data structures used during proof generation
 */

static int hint_id[HINT_COUNT+1];
static int hint_buf[HINT_COUNT+1][MAX_CLAUSE+ILIST_OVHD];
static ilist hint_clause[HINT_COUNT+1];
static bool hint_used[HINT_COUNT+1];

static jtype_t hint_hl_order[HINT_COUNT] = 
    { HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH, HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL };

static jtype_t hint_lh_order[HINT_COUNT] = 
    { HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL, HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH };

static jtype_t hint_h_order[HINT_COUNT/2] = 
    { HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH };

/* This one gets used in second half of split proof */
static jtype_t hint_l_order[HINT_COUNT/2+1] = 
    { HINT_EXTRA, HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL };


static char hstring[1024];

static void initialize_hints() {
    jtype_t hi;
    for (hi = (jtype_t) 0; hi < HINT_COUNT+1; hi++) {
	hint_id[hi] = TAUTOLOGY;
	hint_clause[hi] = ilist_make(hint_buf[hi], 3);
    }
}

static void complete_hints() {
    jtype_t hi;
    for (hi = (jtype_t) 0; hi < HINT_COUNT+1; hi++) {
	if (hint_id[hi] == TAUTOLOGY)
	    hint_clause[hi] = TAUTOLOGY_CLAUSE;
	else {
	    hint_clause[hi] = clean_clause(hint_clause[hi]);
	    if (hint_clause[hi] == TAUTOLOGY_CLAUSE)
		hint_id[hi] = TAUTOLOGY;
	}
    }
}

static void show_hints() {
    jtype_t hi;
    for (hi = (jtype_t) 0; hi < HINT_COUNT+1; hi++) {
	if (hint_id[hi] != TAUTOLOGY) {
	    fprintf(proof_file, "c    %s: #%d = [", hint_name[hi], hint_id[hi]);
	    ilist_print(hint_clause[hi], proof_file, " ");
	    fprintf(proof_file, "]\n");
	}
    }
}


static ilist target_and(ilist ils, BDD l, BDD r, BDD s) {
    return ilist_fill3(ils, -XVAR(l), -XVAR(r), XVAR(s));
}

static ilist target_imply(ilist ils, BDD l, BDD r) {
    return ilist_fill2(ils, -XVAR(l), XVAR(r));
}
	

static bool rup_check(ilist target_clause, jtype_t *horder, int hcount) {
    int ubuf[8+ILIST_OVHD];
    ilist ulist = ilist_make(ubuf, 8);
    int cbuf[MAX_CLAUSE+ILIST_OVHD];
    ilist cclause = ilist_make(cbuf, MAX_CLAUSE);
    int oi, hi, li, ui;
    for (ui = 0; ui < ilist_length(target_clause); ui++)
	ilist_push(ulist, -target_clause[ui]);
    if (verbosity_level >= 4) {
	fprintf(proof_file, "c RUP start.  Target = [");
	ilist_print(target_clause, proof_file, " ");
	fprintf(proof_file, "]\n");
    }
    for (hi = 0; hi < HINT_COUNT; hi++) 
	hint_used[hi] = false;
    for (oi = 0; oi < hcount; oi++) {
	jtype_t hi = horder[oi];
	if (hint_id[hi] != TAUTOLOGY) {
	    ilist clause = hint_clause[hi];
	    /* Operate on copy of clause so that can manipulate */
	    ilist_resize(cclause, 0);
	    for (li = 0; li < ilist_length(clause); li++)
		ilist_push(cclause, clause[li]);
	    if (verbosity_level >= 4) {
		fprintf(proof_file, "c   RUP step.  Units = [");
		ilist_print(ulist, proof_file, " ");
		fprintf(proof_file, "] Clause = %s\n", hint_name[hi]);
	    }
	    li = 0;
	    while (li < ilist_length(cclause)) {
		int lit = cclause[li];
		if (verbosity_level >= 5) {
		    fprintf(proof_file, "c     cclause = [");
		    ilist_print(cclause, proof_file, " ");
		    fprintf(proof_file, "]  ");
		}
		bool found = false;
		for (ui = 0; ui < ilist_length(ulist); ui++) {
		    if (lit == -ulist[ui]) {
			found = true;
			break;
		    }
		    if (lit == ulist[ui]) {
			if (verbosity_level >= 5)
			    fprintf(proof_file, "Unit %d Found.  Creates tautology\n", -lit);
			return false;
		    }
		}
		if (found) { 
		    if (verbosity_level >= 5)
			fprintf(proof_file, "Unit %d found.  Deleting %d\n", -lit, lit);
		    if (ilist_length(cclause) == 1) {
			print_proof_comment(4, "   Conflict detected");
			/* Conflict detected */
			hint_used[hi] = true;
			return true;
		    } else {
			/* Remove lit from cclause by swapping with last one */
			int nlength = ilist_length(cclause)-1;
			cclause[li] = cclause[nlength];
			ilist_resize(cclause, nlength);
		    }
		} else {
		    if (verbosity_level >= 5)
			fprintf(proof_file, "Unit %d NOT found.  Keeping %d\n", -lit, lit);
		    li++;
		}
	    }
	    if (ilist_length(cclause) == 1) {
		/* Unit propagation */
		print_proof_comment(5, "  Unit propagation of %d", cclause[0]);
		ilist_push(ulist, cclause[0]);
		hint_used[hi] = true;
	    }
	}
    }
    /* Didn't find conflict */
    print_proof_comment(4, "  RUP failed");
    return false;
}



int justify_apply(int op, BDD l, BDD r, int splitVar, TBDD tresl, TBDD tresh, BDD res) {
    int tbuf[MAX_CLAUSE+ILIST_OVHD];
    ilist targ = ilist_make(tbuf, MAX_CLAUSE);
    int itbuf[MAX_CLAUSE+ILIST_OVHD];
    ilist itarg = ilist_make(itbuf, MAX_CLAUSE);
    int abuf[8+ILIST_OVHD];
    ilist ant = ilist_make(abuf, 8);
    int oi, hi, li;

    int jid = 0;
    if (op == bddop_andj) {
	targ = clean_clause(target_and(targ, l, r, res));
	print_proof_comment(2, "Generating proof that N%d & N%d --> N%d", bdd_nameid(l), bdd_nameid(r), bdd_nameid(res));
	print_proof_comment(3, "splitVar = %d, tresl.root = N%d, tresh.root = N%d", splitVar, bdd_nameid(tresl.root), bdd_nameid(tresh.root));
    } else {
	targ = clean_clause(target_imply(targ, l, r));
	print_proof_comment(2, "Generating proof that N%d --> N%d", bdd_nameid(l), bdd_nameid(r));
	print_proof_comment(3, "splitVar = %d", splitVar);
    }
    if (targ == TAUTOLOGY_CLAUSE) {
	print_proof_comment(2, "Tautology");
	return TAUTOLOGY;
    }
    if (verbosity_level >= 3) {
	fprintf(proof_file, "Target clause = [");
	ilist_print(targ, proof_file, " ");
	fprintf(proof_file, "]\n");
    }
    

    /* Prepare the candidates */
    initialize_hints();

    if (LEVEL(l) == splitVar) {
	hint_id[HINT_ARG1LD] = bdd_dclause(l, DEF_LD);
	hint_clause[HINT_ARG1LD] = defining_clause(hint_clause[HINT_ARG1LD], DEF_LD, XVAR(l), splitVar, XVAR(HIGH(l)), XVAR(LOW(l)));
	hint_id[HINT_ARG1HD] = bdd_dclause(l, DEF_HD);
	hint_clause[HINT_ARG1HD] = defining_clause(hint_clause[HINT_ARG1HD], DEF_HD, XVAR(l), splitVar, XVAR(HIGH(l)), XVAR(LOW(l)));
    }

    BDD ll = LEVEL(l) == splitVar ? LOW(l) : l;
    BDD lh = LEVEL(l) == splitVar ? HIGH(l) : l;
    BDD rl = LEVEL(r) == splitVar ? LOW(r) : r;
    BDD rh = LEVEL(r) == splitVar ? HIGH(r) : r;

    if (op == bddop_imptstj) {
	if (LEVEL(r) == splitVar) {
	    hint_id[HINT_RESLU] = bdd_dclause(r, DEF_LU);
	    hint_clause[HINT_RESLU] = defining_clause(hint_clause[HINT_RESLU], DEF_LU, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	    hint_id[HINT_RESHU] = bdd_dclause(r, DEF_HU);
	    hint_clause[HINT_RESHU] = defining_clause(hint_clause[HINT_RESHU], DEF_HU, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	}
	hint_id[HINT_OPL] = ABS(tresl.clause_id);
	hint_clause[HINT_OPL] = target_imply(hint_clause[HINT_OPL], ll, rl);
	hint_id[HINT_OPH] = ABS(tresh.clause_id);
	hint_clause[HINT_OPH] = target_imply(hint_clause[HINT_OPH], lh, rh);
    } else {
	if (LEVEL(r) == splitVar) {
	    hint_id[HINT_ARG2LD] = bdd_dclause(r, DEF_LD);
	    hint_clause[HINT_ARG2LD] = defining_clause(hint_clause[HINT_ARG2LD], DEF_LD, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	    hint_id[HINT_ARG2HD] = bdd_dclause(r, DEF_HD);
	    hint_clause[HINT_ARG2HD] = defining_clause(hint_clause[HINT_ARG2HD], DEF_HD, XVAR(r), splitVar, XVAR(HIGH(r)), XVAR(LOW(r)));
	}
	if (tresl.root != tresh.root) {
	    hint_id[HINT_RESLU] = bdd_dclause(res, DEF_LU);
	    hint_clause[HINT_RESLU] = defining_clause(hint_clause[HINT_RESLU], DEF_LU, XVAR(res), splitVar, XVAR(HIGH(res)), XVAR(LOW(res)));
	    hint_id[HINT_RESHU] = bdd_dclause(res, DEF_HU);
	    hint_clause[HINT_RESHU] = defining_clause(hint_clause[HINT_RESHU], DEF_HU, XVAR(res), splitVar, XVAR(HIGH(res)), XVAR(LOW(res)));
	}
	hint_id[HINT_OPL] = ABS(tresl.clause_id);
	hint_clause[HINT_OPL] = target_and(hint_clause[HINT_OPL], ll, rl, tresl.root);
	hint_id[HINT_OPH] = ABS(tresh.clause_id);
	hint_clause[HINT_OPH] = target_and(hint_clause[HINT_OPH], lh, rh, tresh.root);
    }

    complete_hints();
    if (verbosity_level >= 3) {
	print_proof_comment(3, "Hints:");
	show_hints();
    }

    bool checked = false;
    if (hint_id[HINT_OPH] == TAUTOLOGY) {
	/* Try for single clause proof */
	if (rup_check(targ, hint_hl_order, HINT_COUNT)) {
	    checked = true;
	    for (oi = 0; oi < HINT_COUNT; oi++) {
		hi = hint_hl_order[oi];
		if (hint_used[hi])
		    ilist_push(ant, hint_id[hi]);
	    }
	    jid = generate_clause(targ, ant);
	}

    }
    if (!checked && hint_id[HINT_OPL] == TAUTOLOGY) {
	if (rup_check(targ, hint_lh_order, HINT_COUNT)) {
	    checked = true;
	    for (oi = 0; oi < HINT_COUNT; oi++) {
		hi = hint_lh_order[oi];
		if (hint_used[hi])
		    ilist_push(ant, hint_id[hi]);
	    }
	    jid = generate_clause(targ, ant);
	}
    }
    if (!checked) {
	ilist_push(itarg, -splitVar);
	for (li = 0; li < ilist_length(targ); li++)
	    ilist_push(itarg, targ[li]);
	itarg = clean_clause(itarg);
	if (!rup_check(itarg, hint_h_order, HINT_COUNT/2)) {
	    fprintf(proof_file, "c  Uh-Oh.  RUP check failed in first half of proof.  Target = [");
	    ilist_print(itarg, proof_file, " ");
	    fprintf(proof_file, "].  SKIPPING\n");
	    // exit(1);
	}
	for (oi = 0; oi < HINT_COUNT/2; oi++) {
	    hi = hint_h_order[oi];
	    if (hint_used[hi])
		ilist_push(ant, hint_id[hi]);
	}
	int iid = generate_clause(itarg, ant);
	hint_id[HINT_EXTRA] = ABS(iid);
	hint_clause[HINT_EXTRA] = itarg;
	if (!rup_check(targ, hint_l_order, HINT_COUNT/2+1)) {
	    fprintf(proof_file, "c  Uh-Oh.  RUP check failed in second half of proof.  Target = [");
	    ilist_print(itarg, proof_file, " ");
	    fprintf(proof_file, "].  SKIPPING\n");
	    //	    exit(1);

	}
	ilist_resize(ant, 0);
	for (oi = 0; oi < HINT_COUNT/2+1; oi++) {
	    hi = hint_l_order[oi];
	    if (hint_used[hi])
		ilist_push(ant, hint_id[hi]);
	}
	// Negate ID to show that two clauses were generated
	jid = -generate_clause(targ, ant);
    }
    return jid;
}


#if 0
/* Initialize set of hints with TAUTOLOGY, and the hint clauses to empty clauses  */
void fill_hints(jtype_t hint_id[HINT_COUNT], int hint_buf[HINT_COUNT][3+ILIST_OVHD], ilist hint_clause[HINT_COUNT]) {
    jtype_t i;
    for (i = (jtype_t) 0; i < HINT_COUNT; i++) {
	hint_id[i] = TAUTOLOGY;
	hint_clause[i] = ilist_make(hint_buf[i], 3);
    }
}


/* Justify results of apply operation.  Return clause ID */
int justify_apply(ilist target_clause, int split_variable, jtype_t hint_id[HINT_COUNT], ilist hint_clause[HINT_COUNT]) {
    int abuf[HINT_COUNT+1+ILIST_OVHD];
    ilist ant = ilist_make(abuf, HINT_COUNT);
    target_clause = clean_clause(target_clause);
    if (target_clause == TAUTOLOGY_CLAUSE)
	return TAUTOLOGY;
    jtype_t i;
    for (i = (jtype_t) 0; i < HINT_COUNT; i++) {
	if (hint_id[i] == TAUTOLOGY)
	    hint_clause[i] = TAUTOLOGY_CLAUSE;
	else {
	    hint_clause[i] = clean_clause(hint_clause[i]);
	    if (hint_clause[i] == TAUTOLOGY_CLAUSE)
		hint_id[i] = TAUTOLOGY;
	    else
		ilist_push(ant, hint_id[i]);
	}
    }
    return generate_clause(target_clause, ant);
}
#endif


