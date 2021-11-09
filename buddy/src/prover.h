/* Routines to manage proof generation */

#ifndef PROVER_H
#define PROVER_H

#include "tbdd.h"

#include <stdarg.h>

/* Return clause ID */
/* For DRAT proof, antecedents should be NULL */
int generate_clause(ilist literals, ilist antecedent);
void delete_clauses(ilist clause_ids);

/* 
   Fill Convert buf to ilist and fill with defining clause
   dbuf should be at least 3+ILIST_OVHD long 
   Return either the converted buffer or TAUTOLOGY_CLAUSE 
*/
ilist defining_clause(int *buf, dclause_t dtype, int vid, int hid, int lid);

/*
  Fix clause by removing redundant literals and tautologies.
  Detect if clause is tautological
  Will return either argument ilist or CLAUSE_TAUTOLOGY
*/
ilist clean_clause(ilist clause);

/*
  Print comment in proof. Don't need to include "c" in string.
  Will put newline at end if not already there
 */

void print_proof_comment(char *fmt, ...);

#endif /* PROVER_H */

/* EOF */
