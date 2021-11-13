/* Routines to manage proof generation */

#ifndef PROVER_H
#define PROVER_H

#include "tbdd.h"
#include <stdarg.h>

/* Allow this headerfile to define C++ constructs if requested */
#ifdef __cplusplus
#define CPLUSPLUS
#endif

#ifdef CPLUSPLUS
extern "C" {
#endif


/* Global variables exported by prover */
extern bool do_lrat;
extern int verbosity_level;
extern int last_variable;
extern int last_clause_id;
extern int total_clause_count;
extern int input_variable_count;

/* Prover setup and completion */
extern int prover_init(FILE *pfile, int input_variable_count, int input_clause_count, ilist *clauses, bool lrat);
extern void prover_done();

/* Return clause ID */
/* For DRAT proof, antecedents can be NULL */
extern int generate_clause(ilist literals, ilist antecedent);
extern void delete_clauses(ilist clause_ids);

/* Retrieve input clause.  NULL if invalid */
extern ilist get_input_clause(int id);

/* 
   Fill ilist with defining clause.
   ils should be big enough for 3 elemeents.
   Return either the list of literals or TAUTOLOGY_CLAUSE 
*/
extern ilist defining_clause(ilist ils, dclause_t dtype, int nid, int vid, int hid, int lid);

/* Print clause */
extern void print_clause(FILE *out, ilist clause);

/*
  Print comment in proof. Don't need to include "c" in string.
  Only will print when verbosity level at least that specified
  Newline printed at end
 */

extern void print_proof_comment(int vlevel, char *fmt, ...);

/*
  Support for generating apply operation proofs.
 */

/*
  Enumerated type for the hint types
 */
typedef enum { HINT_RESHU, HINT_ARG1HD, HINT_ARG2HD, HINT_OPH, HINT_RESLU, HINT_ARG1LD, HINT_ARG2LD, HINT_OPL, HINT_COUNT } jtype_t;

/* In the following, hints should be an array of size HINT_COUNT */

/* Initialize set of hints with TAUTOLOGY */
extern void fill_hints(jtype_t *hints);

/* Justify results of apply operation.  Return clause ID */
extern int justify_apply(ilist target_clause, int split_variable, jtype_t *hints);

#ifdef CPLUSPLUS
}
#endif

#endif /* PROVER_H */

/* EOF */
