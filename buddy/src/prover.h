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
extern int max_live_clause_count;

/* Prover setup and completion */
extern int prover_init(FILE *pfile, int input_variable_count, int input_clause_count, ilist *clauses, bool lrat);
extern void prover_done();

/* Put literals in clause in canonical order */
extern ilist clean_clause(ilist clause);

/* Return clause ID */
/* For DRAT proof, antecedents can be NULL */
extern int generate_clause(ilist literals, ilist antecedent);

extern void delete_clauses(ilist clause_ids);

/* Some deletions must be deferred until top-level apply completes */
extern void defer_delete_clause(int clause_id);
extern void process_deferred_deletions();


    
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
  Determine whether information can be printed to proof file.
  Do so only when printing LRAT format and verbosity level is sufficiently high
 */
extern bool print_ok(int vlevel);

/*
  Print comment in proof. Don't need to include "c" in string.
  Only will print when verbosity level at least that specified
  Newline printed at end
 */

extern void print_proof_comment(int vlevel, const char *fmt, ...);

/*
  Support for generating apply operation proofs.
 */

    
/* Complete proof of apply operation */
/* Absolute of returned value indicates the ID of the justifying proof step */
/* Value will be < 0 when previous clause ID also used as intermediate step */
extern int justify_apply(int op, BDD l, BDD r, int splitVar, TBDD tresl, TBDD tresh, BDD res);


#ifdef CPLUSPLUS
}
#endif

#endif /* PROVER_H */

/* EOF */
