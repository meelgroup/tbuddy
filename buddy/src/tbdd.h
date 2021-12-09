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

#ifndef CPLUSPLUS
typedef TBDD tbdd;
#endif

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

  Return 0 if OK, otherwise error code
*/

extern int tbdd_init(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses, bool lrat);

/* 
   Initializers specific for the two proof formats
 */
extern int tbdd_init_lrat(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses);
extern int tbdd_init_drat(FILE *pfile, int variable_count);


/*
  Deallocate all resources used by TBDD.
  If verbosity >= 1, print summary information
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

extern TBDD tbdd_from_clause(ilist clause);  // For DRAT
extern TBDD tbdd_from_clause_id(int id);     // For LRAT

/*
  Generate BDD representation of XOR.
  For DRAT
 */
extern TBDD TBDD_from_xor(ilist variables, int phase);

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
  Form conjunction of TBDDs tl & tr.  Use to validate
  BDD r
 */
extern TBDD tbdd_validate_with_and(BDD r, TBDD tl, TBDD tr);

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


#ifdef CPLUSPLUS
}
#endif

#ifndef CPLUSPLUS
#define tbdd_tautology TBDD_tautology
#define tbdd_from_xor TBDD_from_xor
#define tbdd_null TBDD_null
#endif

#ifdef CPLUSPLUS
/*============================================
 C++ interface
============================================*/
namespace trustbdd {
class tbdd
{
 public:

    //    tbdd(const BDD &r, const int &id) { bdd_addref(root=r); clause_id=id; }
    tbdd(const bdd &r, const int &id) { bdd_addref(root=r.get_BDD()); clause_id=id; }
    tbdd(const tbdd &t)               { bdd_addref(root=t.root); clause_id=t.clause_id; }
    tbdd(TBDD tr)                     { bdd_addref(root=tr.root); clause_id=tr.clause_id; }
    tbdd(ilist clause)                { tbdd(tbdd_from_clause(clause)) ; }
    tbdd(int id)                      { tbdd(tbdd_from_clause_id(id)) ; }
    tbdd(void)                        { root=1; clause_id = TAUTOLOGY; }

    ~tbdd(void)                       { TBDD dr; dr.root = root; dr.clause_id = clause_id; tbdd_delref(dr); }

    tbdd operator=(const tbdd &tr)    { if (root != tr.root) { TBDD dr; dr.root = root; dr.clause_id = clause_id; tbdd_delref(dr); root = tr.root; bdd_addref(root); } clause_id = tr.clause_id; return *this; }
    tbdd operator&(const tbdd &tr) const;
    tbdd operator&=(const tbdd &tr);
    
    // Backdoor functions provide read-only access
    bdd get_root()                     { return bdd(root); }
    int get_clause_id()                { return clause_id; }

 private:
    BDD root;
    int clause_id;  /* Id of justifying clause */

    friend tbdd tbdd_tautology(void);
    friend tbdd tbdd_null(void);
    friend bool tbdd_is_false(tbdd &tr);
    friend tbdd tbdd_and(tbdd &tl, tbdd &tr);
    friend tbdd tbdd_validate(bdd r, tbdd &tr);
    friend tbdd tbdd_validate_with_and(bdd r, tbdd &tl, tbdd &tr);
    friend tbdd tbdd_trust(bdd r);
    friend int tbdd_validate_clause(ilist clause, tbdd &tr);
    friend tbdd tbdd_from_xor(ilist variables, int phase);
    friend int tbdd_nameid(tbdd &tr);

    // Convert to low-level form
    friend void tbdd_xfer(tbdd &tr, TBDD &res);
};

inline tbdd tbdd_tautology(void)
{ return tbdd(TBDD_tautology()); }

inline tbdd tbdd_null(void)
{ return tbdd(TBDD_null()); }


inline bool tbdd_is_false(tbdd &tr)
{ return tr.root == bdd_false().get_BDD(); }

inline tbdd tbdd_and(tbdd &tl, tbdd &tr)
{ TBDD TL, TR; tbdd_xfer(tl, TL); tbdd_xfer(tr, TR); return tbdd(tbdd_and(TL, TR)); }

inline tbdd tbdd_validate(bdd r, tbdd &tr)
{ TBDD TR; tbdd_xfer(tr, TR); return tbdd(tbdd_validate(r.get_BDD(), TR)); }

inline tbdd tbdd_validate_with_and(bdd r, tbdd &tl, tbdd &tr)
{ TBDD TL, TR; tbdd_xfer(tl, TL); tbdd_xfer(tr, TR); return tbdd(tbdd_validate_with_and(r.get_BDD(), TL, TR)); }

inline tbdd tbdd_trust(bdd r)
{ return tbdd(tbdd_trust(r.get_BDD())); }

inline void tbdd_xfer(tbdd &tr, TBDD &res)
{ res.root = tr.root; res.clause_id = tr.clause_id; }

inline int tbdd_validate_clause(ilist clause, tbdd &tr)
{ TBDD TR; tbdd_xfer(tr, TR); return tbdd_validate_clause(clause, TR); }

inline tbdd tbdd_from_xor(ilist variables, int phase)
{ return TBDD_from_xor(variables, phase); }

inline int tbdd_nameid(tbdd &tr)
{ return bdd_nameid(tr.root); }

} /* Namespace trustbdd */
#endif /* CPLUSPLUS */

#endif /* _TBDD_H */
/* EOF */


