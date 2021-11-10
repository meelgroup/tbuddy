#include <stdlib.h>
#include <string.h>
#include "prover.h"


/* Prover data */
static FILE *proof_file = NULL;
static int next_var = 1;
static int next_clause_id = 1;
static int input_clause_count = 0;

/* Helper function for clause cleaning.  Sort literals so that pos & neg literals are interleaved */
/* Want variables in descending order */
int literal_compare(const void *l1p, const void *l2p) {
     int l1 = *(int *) l1p;
     int l2 = *(int *) l2p;
     int o1 = l1 < 0 ? 2*-l1 : 2*l1+1;
     int o2 = l2 < 0 ? 2*-l2 : 2*l2;
     if (o2 > o1)
	 return 1;
     if (o2 < o1)
	 return -1;
     return 0;
}

ilist clean_clause(ilist clause) {    
    int len = ilist_length(clause);
    if (len == 0)
	return clause;
    /* Sort the literals */
    qsort((void *) ilist, ilist_length(ilist), sizeof(int), literal_compare);
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
    return ilist_resize(ilist, puti);
}

