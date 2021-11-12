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

/* Global variables used by prover */
static FILE *proof_file = NULL;
/* 
   For LRAT, only need to keep dictionary of input clauses.
   For DRAT, need dictionary of all clauses in order to delete them.
*/

static ilist *all_clauses = NULL;
static int input_clause_count = 0;
static int alloc_clause_count = 0;

// Parameters
// Cutoff betweeen large and small allocations (in terms of clauses)
#define BUDDY_THRESHOLD 1000
//#define BUDDY_THRESHOLD 10
#define BUDDY_NODES_LARGE (100*1000*1000)
//#define BUDDY_NODES_LARGE (1000)
#define BUDDY_NODES_SMALL (    1000*1000)
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

ilist clean_clause(ilist clause) {
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

/* Return clause ID */
/* For DRAT proof, antecedents can be NULL */
int generate_clause(ilist literals, ilist antecedent) {
    ilist clause = clean_clause(literals);
    int cid = ++last_clause_id;
    if (clause == TAUTOLOGY_CLAUSE)
	return TAUTOLOGY;
    if (proof_file == NULL) {
	fprintf(stderr, "Invalid proof file!.  Switching to stdout\n");
	proof_file = stdout;
    }
    if (do_lrat)
	fprintf(proof_file, "%d ", cid);
    ilist_print(clause, proof_file, " ");
    if (do_lrat) {
	fprintf(proof_file, " 0 ");
	ilist_print(antecedent, proof_file, " ");
    }
    fprintf(proof_file, " 0\n");
    total_clause_count++;
    
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

