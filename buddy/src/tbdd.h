
#ifndef _TBDD_H
#define _TBDD_H

#include <limits.h>
#include "bdd.h"
#include "ilist.h"

/*============================================
API for trusted extension to BuDDy BDD package
============================================*/

/* Allow this headerfile to define C++ constructs if requested */
#ifdef __cplusplus
#define CPLUSPLUS
#endif


/* Value representing logical truth */
#define TAUTOLOGY INT_MAX 

#ifdef CPLUSPLUS
extern "C" {
#endif

/* 
   A trusted BDD is one for which a proof has
   been generated showing that it is logically
   entailed by a set of input clauses.
*/
typedef struct {
    BDD root;
    int clause_id;  /* Id of justifying clause */
} TBDD;

//#ifndef CPLUSPLUS
typedef TBDD tbdd;
//#endif


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
extern tbdd tbdd_addref(tbdd tr);
extern void tbdd_delref(tbdd tr);

/* 
   proof_step = TAUTOLOGY
   root = 1
 */
extern tbdd tbdd_tautology();

/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.

  clause_id should be number between 1 and the number of input clauses
 */
extern tbdd tbdd_from_clause(ilist clause);
extern tbdd tbdd_from_clause_id(int id);

/*
  Upgrade ordinary BDD to TBDD by proving
  implication from another TBDD
 */
extern tbdd tbdd_validate(BDD r, tbdd tr);

/*
  Declare BDD to be trustworthy.  Proof
  checker must provide validation.
  Only works when generating DRAT proofs
 */
extern tbdd tbdd_trust(BDD r);

/*
  Form conjunction of two TBDDs and prove
  their conjunction implies the new one
 */
extern tbdd tbdd_and(tbdd tr1, tbdd tr2);

/*
  Validate that a clause is implied by a TBDD.
  Use this version when generating LRAT proofs
  Returns clause id.
 */
extern int tbdd_validate_clause(ilist clause, tbdd tr);

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
extern BDD bdd_build_xor(ilist variables, int phase);

#ifdef CPLUSPLUS
}
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


