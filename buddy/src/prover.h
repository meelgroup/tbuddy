/* Routines to manage proof generation */

#ifndef PROVER_H
#define PROVER_H

#include "tbdd.h"
#include <stdarg.h>

/* Global variables exported by prover */
extern bool do_lrat;
extern int verbosity_level;
extern int last_variable;
extern int last_clause_id;
extern int total_clause_count;
extern int input_variable_count;

/* Prover setup and completion */
int prover_init(FILE *pfile, int input_variable_count, int input_clause_count, ilist *clauses, bool lrat);
void prover_done();

/* Return clause ID */
/* For DRAT proof, antecedents can be NULL */
int generate_clause(ilist literals, ilist antecedent);
void delete_clauses(ilist clause_ids);

/* Retrieve input clause.  NULL if invalid */
ilist get_input_clause(int id);

/* 
   Fill ilist with defining clause.
   ils should be big enough for 3 elemeents.
   Return either the list of literals or TAUTOLOGY_CLAUSE 
*/
ilist defining_clause(ilist ils, dclause_t dtype, int nid, int vid, int hid, int lid);

/*
  Fix clause by removing redundant literals and tautologies.
  Detect if clause is tautological
  Will return either argument ilist or CLAUSE_TAUTOLOGY
*/
ilist clean_clause(ilist clause);

/* Print clause */
void print_clause(FILE *out, ilist clause);

/*
  Print comment in proof. Don't need to include "c" in string.
  Only will print when verbosity level at least that specified
  Newline printed at end
 */

void print_proof_comment(int vlevel, char *fmt, ...);



#endif /* PROVER_H */

/* EOF */
