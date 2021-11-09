#include <stdlib.h>
#include <string.h>
#include "prover.h"


/* Prover data */
static FILE *proof_file = NULL;
static int next_var = 1;
static int next_clause_id = 1;
static int input_clause_count = 0;


