/* Interface for proof-generating operations on Pseudo-Boolean functions */
/* Initial version on supports XOR constraints */

#ifndef _PSEUDOBOOLEAN_H
#define _PSEUDOBOOLEAN_H

#include "tbdd.h"

class xor_constraint {
 private:
    ilist variables;
    int phase;
    tbdd validation;

 public:
    xor_constraint() { variables = ilist_new(0); phase = 0; validation = tbdd_tautology(); }
    xor_constraint(ilist vars, int p, tbdd &vfun);
    xor_constraint(xor_constraint &x) { variables = ilist_copy(x.variables); phase = x.phase; validation = x.validation; }
    xor_constraint(ilist variables, int phase);
    
    ~xor_constraint(void) { ilist_free(variables); variables = NULL; }

    bool is_feasible(void) { return ilist_length(variables) > 0 || phase == 0; }

    int validate_clause(ilist clause);

    void accum(xor_constraint &other);

    xor_constraint plus(xor_constraint &other);

    tbdd get_validation() { return validation; }

    void show(FILE *out);

    friend xor_constraint *sum_list(xor_constraint **xlist, int len);
};


#endif /* PSEUDOBOOLEAN */
