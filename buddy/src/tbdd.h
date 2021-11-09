
#ifndef _TBDD_H
#define _TBDD_H

#include <limits.h>
#include "bdd.h"

/*============================================
API for trusted extension to BuDDy BDD package
============================================*/


/* Value representing logical truth */
#define TAUTOLOGY INT_MAX 

/* 
   A trusted BDD is one for which a proof has
   been generated showing that it is logically
   entailed by a set of input clauses.
*/
typedef struct {
    BDD root;
    int clause_id;  /* Id of justifying clause */
} TBDD;

#ifndef CPLUSPLUS
typedef TBDD tbdd;
#endif


/*============================================
   Integer lists
============================================*/

/*
  Data type ilist is used to represent clauses and clause id lists.
  These are simply lists of integers, where the value at position -1
  indicates the list length and the value at position -2 indicates the
  maximum list length.  The value at position -2 is positive for
  statically-allocated ilists and negative for ones that can be
  dynamically resized.
*/
typedef int *ilist;
  
/*
  Difference between ilist maximum length and number of allocated
  integers
 */
#define ILIST_OVHD 2

/*
  Pseudo-clause representing tautology
 */
#define TAUTOLOGY_CLAUSE ((ilist) TAUTOLOGY)

/* 
   Convert an array of ints to an ilist.  Don't call free_ilist on
   this one!  The size of the array should be max_length + ILIST_OVHD
   Will be statically sized
*/
ilist ilist_make(int *p, int max_length);

/* Allocate a new ilist. */
ilist ilist_new(int max_length);

/* Free allocated ilist.  Only call on ones allocated via ilist_new */
void ilist_free();

/* Return number of elements in ilist */
int ilist_length(ilist ils);

/*
  Add new value(s) to end of ilist.
  For dynamic ilists, the value of the pointer may change
*/
ilist ilist_push(ilist ils, int val);

/*
  Populate ilist with 1, 2, 3, or 4 elements.
  For dynamic ilists, the value of the pointer may change
 */
ilist ilist_fill1(ilist *ils, int val1);
ilist ilist_fill2(ilist *ils, int val1, int val2);
ilist ilist_fill3(ilist *ils, int val1, int val2, int val3);
ilist ilist_fill4(ilist *ils, int val1, int val2, int val3, int val4);

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

int tbdd_init_drat(FILE *pfile, int varcount);
int tbdd_init_lrat(FILE *pfile, int varcount, int clause_count, ilist *clauses);

/*
  Deallocate all resources used by TBDD
 */
void tbdd_done();


/*============================================
 Creation and manipulation of trusted BDDs
============================================*/

/* 
   proof_step = TAUTOLOGY
   root = 1
 */
tbdd tbdd_tautology();

/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.

  Only possible when generating LRAT proofs
 */
tbdd tbdd_from_clause(int clause_id);

/*
  Upgrade ordinary BDD to TBDD by proving
  implication from another TBDD
 */
tbdd tbdd_validate(bdd r, tbdd tr);

/*
  Declare BDD to be trustworthy.  Proof
  checker must provide validation.
  Only works when generating DRAT proofs
 */
tbdd tbdd_trust(bdd r);

/*
  Form conjunction of two TBDDs and prove
  their conjunction implies the new one
 */
tbdd tbdd_and(tbdd tr1, tbdd tr2);

/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */
int tbdd_validate_clause(ilist clause, tbdd tr);

/*
  Assert that a clause holds.  Proof checker
  must provide validation.
  Use this version when generating DRAT proofs
 */
void assert_clause(ilist clause);

/*============================================
 Useful operations on BDDs
============================================*/

/*
  Build BDD representation of XOR (phase = 1) or XNOR (phase = 0)
 */
bdd bdd_build_xor(ilist variables, int phase);

#endif /* _TBDD_H */
/* EOF */
