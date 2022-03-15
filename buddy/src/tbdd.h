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

/*
   A trusted BDD is one for which a proof has
   been generated showing that it is logically
   entailed by a set of input clauses.

   Trusted BDDs have:
      A root BDD node
      A clause Id for the justifying clause
      A reference count, distinct from that of the BDD node's reference count

  A trusted BDD is identified by an index into the trust table 

  A trusted BDD must have a reference count >= 1, or else
  its validating clause will be deleted.
  Consequently, all operations that a return a TBDD
  have an incremented reference count.  

  A trusted BDD maintains a single reference to the root BDD.  When
  the trusted BDD's reference count is decremented to zero, the BDD
  reference is decremented

*/
typedef int TBDD;

/* Special TBDDs: */
#define TBDD_NULL 0 
#define TBDD_TAUTOLOGY 1

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

extern int tbdd_init(FILE *pfile, int *variable_counter, int *clause_id_counter, ilist *input_clauses, ilist variable_ordering, proof_type_t ptype, bool binary);

/* 
   Initializers specific for the seven combinations of proof formats
 */
extern int tbdd_init_lrat(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses, ilist variable_ordering);
extern int tbdd_init_lrat_binary(FILE *pfile, int variable_count, int clause_count, ilist *input_clauses, ilist variable_ordering);
extern int tbdd_init_drat(FILE *pfile, int variable_count);
extern int tbdd_init_drat_binary(FILE *pfile, int variable_count);
extern int tbdd_init_frat(FILE *pfile, int *variable_counter, int *clause_id_counter);
extern int tbdd_init_frat_binary(FILE *pfile, int *variable_counter, int *clause_id_counter);
extern int tbdd_init_noproof(int variable_count);

/*
  Deallocate all resources used by TBDD.
  If verbosity >= 1, print summary information
 */
extern void tbdd_done();

/*
  Functions that can be added to provide more data on exit.
  Function takes verbosity level as argument.
 */
typedef void (*tbdd_info_fun)(int);

void tbdd_add_info_fun(tbdd_info_fun f);

typedef void (*tbdd_done_fun)(void);

void tbdd_add_done_fun(tbdd_done_fun f);

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
  Get or generate a new TBDD.  Antecedents used to 
  generate unit clause in proof.
  Reference for root incremented.
  If new trust node created, generates reference to root node
 */
TBDD tbdd_makenode(BDD root, ilist antecedents);

/*
  Increment reference count for TBDD (not normally needed?)
 */
extern TBDD tbdd_addref(TBDD tr);
/*
  Decrement reference count for TBDD.
  If goes to zero, delete clause
  and decrement reference count for root.
 */
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
   Test whether underlying BDD is 0/1
 */
extern bool tbdd_is_true(TBDD tr);
extern bool tbdd_is_false(TBDD tr);

/*
  Generate BDD representation of specified input clause.
  Generate proof that BDD will evaluate to TRUE
  for all assignments satisfying clause.

  clause_id should be number between 1 and the number of input clauses
 */

extern TBDD TBDD_from_clause(ilist clause);  // For DRAT
extern TBDD TBDD_from_clause_id(int id);     // For LRAT

/*
  Generate BDD representation of XOR.
  For DRAT
 */
extern TBDD TBDD_from_xor(ilist variables, int phase);

/* Operations on TBDDs */
extern TBDD      tbdd_and(TBDD, TBDD);

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
  Use this version when generating DRAT proofs,
  or when don't want to provide antecedent in FRAT proof
  Returns clause id.
 */
extern int assert_clause(ilist clause);

/*============================================
 Useful BDD operations
============================================*/

/* Construct BDD representation of a clause */
extern BDD BDD_build_clause(ilist literals);
/* Construct BDD representation of XOR (phase = 1) or XNOR (phase = 0) */
extern BDD BDD_build_xor(ilist variables, int phase);

/* Represent conjunction of literals (a "cube") as a BDD or an ilist */
extern BDD BDD_build_cube(ilist literals);
extern ilist BDD_decode_cube(BDD r);

#ifdef CPLUSPLUS
}
#endif

#ifndef CPLUSPLUS
#define tbdd_tautology TBDD_tautology
#define tbdd_from_xor TBDD_from_xor
#define tbdd_null TBDD_null
#define tbdd_from_clause TBDD_from_clause
#define tbdd_from_clause_id TBDD_from_clause_id
#endif

#ifdef CPLUSPLUS
/*============================================
 C++ interface
============================================*/
namespace trustbdd {
class tbdd
{
 public:

    tbdd(void) { tnode=TBDD_TAUTOLOGY; }
    tbdd(const tbdd &t) { tnode = tbdd_addref(t.tnode); }
    ~tbdd(void)         { tbdd_delref(tnode); }
    tbdd(TBDD T)                     { tnode = tbdd_addref(T); }

    tbdd operator=(const tbdd &t)    { if (tnode != t.tnode) { tbdd_delref(tnode) ; tnode = t.tnode; tbdd_addref(tnode); } return *this; }
    
    // Backdoor functions provide read-only access
    TBDD get_tnode()                     { return tnode; }

 private:
    TBDD tnode;

    friend tbdd tbdd_makenode(const bdd &r, ilist antecedents);
    friend tbdd tbdd_tautology(void);
    friend tbdd tbdd_null(void);
    friend bool tbdd_is_true(const tbdd &t);
    friend bool tbdd_is_false(const tbdd &t);
    friend void tbdd_delref(const tbdd &t);
    friend tbdd tbdd_and(const tbdd &tl, const tbdd &t);
    friend tbdd tbdd_validate(const bdd r, const tbdd &t);
    friend tbdd tbdd_validate_with_and(const bdd r, const tbdd &tl, const tbdd &tr);
    friend tbdd tbdd_trust(const bdd r);
    friend int tbdd_validate_clause(ilist clause, const tbdd &t);
    friend tbdd tbdd_from_xor(ilist variables, int phase);
    friend int tbdd_nameid(const tbdd &t);
    friend bdd bdd_build_xor(ilist literals);
    friend bdd bdd_build_clause(ilist literals);
    friend bdd bdd_build_cube(ilist literals);
    friend ilist bdd_decode_cube(const bdd &r);
};

inline tbdd tbdd_tautology(void)
{ return tbdd(TBDD_tautology()); }

inline tbdd tbdd_null(void)
{ return tbdd(TBDD_null()); }

inline bool tbdd_is_true(const tbdd &t)
{ return tbdd_is_true(t.tnode); }

inline bool tbdd_is_false(const tbdd &t)
{ return tbdd_is_false(t.tnode); }

 inline void tbdd_delref(const tbdd &t)
 { tbdd_delref(t.tnode); }

inline tbdd tbdd_and(const tbdd &tl, const tbdd &tr)
{ return tbdd(tbdd_and(tl.tnode, tr.tnode)); }

inline tbdd tbdd_validate(const bdd r, const tbdd &t)
{ return tbdd(tbdd_validate(r.get_BDD(), t.tnode)); }

inline tbdd tbdd_validate_with_and(const bdd r, const tbdd &tl, const tbdd &tr)
{ return tbdd(tbdd_validate_with_and(r.get_BDD(), tl.tnode, tr.tnode)); }

inline tbdd tbdd_trust(const bdd r)
{ return tbdd(tbdd_trust(r.get_BDD())); }

inline int tbdd_validate_clause(ilist clause, const tbdd &t)
{ return tbdd_validate_clause(clause, t.tnode); }

inline tbdd tbdd_from_xor(ilist variables, int phase)
{ return tbdd(TBDD_from_xor(variables, phase)); }

inline bdd bdd_build_xor(ilist variables, int phase)
{ return bdd(BDD_build_xor(variables, phase)); }

inline bdd bdd_build_clause(ilist literals)
{ return bdd(BDD_build_clause(literals)); }

inline bdd bdd_build_cube(ilist literals)
{ return bdd(BDD_build_cube(literals)); }

inline ilist bdd_decode_cube(const bdd &r)
{ return bdd_decode_cube(r.get_BDD()); }



} /* Namespace trustbdd */
#endif /* CPLUSPLUS */

#endif /* _TBDD_H */
/* EOF */


