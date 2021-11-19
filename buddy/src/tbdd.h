#ifndef _TBDD_H
#define _TBDD_H

/* Turn on TBDD-specific information in files */
#ifndef ENABLE_TBDD
#define ENABLE_TBDD 1
#endif

/* Allow this headerfile to define C++ constructs if requested */
#ifdef __cplusplus
#define CPLUSPLUS
#endif

/*============================================
 API for trusted extension to BuDDy BDD package
============================================*/


#include <limits.h>
#include "ilist.h"
#include "bdd.h"

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

//#ifdef CPLUSPLUS
typedef TBDD tbdd;
//#endif

#ifdef CPLUSPLUS
extern "C" {
#endif


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

extern int tbdd_init_drat(FILE *pfile, int variable_count, int clause_count);
extern int tbdd_init_lrat(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses);

/*
  Deallocate all resources used by TBDD
 */
extern void tbdd_done();


/*
  Setting optional solver features
 */

/* Verbosity levels:

   0: Totally silent
   1: Summary information (default)
   2: Enable comments in generated proof
   3: Debugging information
*/
extern void tbdd_set_verbose(int level);

/*============================================
 Creation and manipulation of trusted BDDs
============================================*/

/*
  Increment/decrement reference count for BDD
 */
extern TBDD tbdd_addref(TBDD tr);
extern void tbdd_delref(TBDD tr);

/* 
   proof_step = TAUTOLOGY
   root = 1
 */
extern TBDD TBDD_tautology();

/* 
   proof_step = TAUTOLOGY
   root = 0 (Used as an error return)
 */

extern TBDD TBDD_null();

/*
   Test whether underlying BDD is 0
 */
extern bool tbdd_is_false(TBDD tr);

/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.

  clause_id should be number between 1 and the number of input clauses
 */
extern TBDD tbdd_from_clause(ilist clause);
extern TBDD tbdd_from_clause_id(int id);


/* Operations on TBDDs */
extern TBDD      tbdd_and(TBDD, TBDD);

/* Low-level functions to implement operations on TBDDs */
extern TBDD      bdd_andj(BDD, BDD);    
extern TBDD      bdd_imptstj(BDD, BDD);    

/*
  Upgrade ordinary BDD to TBDD by proving
  implication from another TBDD
 */
extern TBDD tbdd_validate(BDD r, TBDD tr);

/*
  Declare BDD to be trustworthy.  Proof
  checker must provide validation.
  Only works when generating DRAT proofs
 */
extern TBDD tbdd_trust(BDD r);

/*
  Form conjunction of two TBDDs and prove
  their conjunction implies the new one
 */
extern TBDD tbdd_and(TBDD tr1, TBDD tr2);

/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */
extern int tbdd_validate_clause(ilist clause, TBDD tr);

/*
  Assert that a clause holds.  Proof checker
  must provide validation.
  Use this version when generating DRAT proofs
 */
extern void assert_clause(ilist clause);

/*============================================
 Useful operations on BDDs
============================================*/

/*
  Build BDD representation of XOR (phase = 1) or XNOR (phase = 0)
 */
extern BDD BDD_build_xor(ilist variables, int phase);

#ifdef CPLUSPLUS
}
#endif

#ifdef CPLUSPLUS
#define tbdd_tautology TBDD_tautology
#define tbdd_null TBDD_null
#else
#define tbdd_tautology TBDD_tautology
#define tbdd_null TBDD_null
#endif

#if 0 /* Save to later */
/*============================================
 C++ interface
============================================*/
#ifdef CPLUSPLUS

class tbdd
{
 public:

    tbdd(void)          { root=1; clause_id=TAUTOLOGY; }
    tbdd(const tbdd &t) { bdd_addref(root=t.root); clause_id=t.clause_id; }
    ~tbdd(void)         { bdd_delref(root); }

 private:
    BDD root;
    int clause_id;  /* Id of justifying clause */

};
#endif /* CPLUSPLUS */
#endif /* DISABLED */

#endif /* _TBDD_H */
/* EOF */


