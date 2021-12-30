/* Interface for proof-generating operations on Pseudo-Boolean functions */
/* Initial version on supports XOR constraints */

#ifndef _PSEUDOBOOLEAN_H
#define _PSEUDOBOOLEAN_H

#include <vector>
#include <cstdint>
#include "tbdd.h"

namespace trustbdd{

// An Xor constraint is represented by a set of variables and a phase
// It also contains a TBDD validation 
class xor_constraint {
 private:
    ilist variables;
    int phase;
    tbdd validation;

 public:
    // Empty constraint represents a tautology
    xor_constraint() { variables = ilist_new(0); phase = 0; validation = trustbdd::tbdd_tautology(); }

    // Construct Xor constraint extracted from product of clauses
    // vfun indicates the TBDD representation of that product
    // For use when generating LRAT proofs
    // The list of variables is used within the constraint and deleted by the Xor constraint destructor
    xor_constraint(ilist vars, int p, trustbdd::tbdd &vfun);

    // Assert that Xor constraint is implied by the clauses
    // For use when generating DRAT proofs
    xor_constraint(ilist vars, int p);

    // Copy an Xor constraint, duplicating the data
    xor_constraint(xor_constraint &x) { variables = ilist_copy(x.variables); phase = x.phase; validation = x.validation; }
    
    // Destructor deletes the list of variables
    ~xor_constraint(void) { ilist_free(variables); variables = NULL; validation = trustbdd::tbdd_null();  }

    // Does the constraint have NO solutions?
    bool is_infeasible(void) { return ilist_length(variables) == 0 || phase != 0; }

    // Does the constraint impose any restrictions on any variables?
    bool is_degenerate(void) { return ilist_length(variables) == 0 && phase == 0; }
    
    // Use xor constraint to validate a clause
    int validate_clause(ilist clause);

    // Get the validation TBDD
    tbdd get_validation() { return validation; }

    ilist get_variables() { return variables; }

    int get_phase() { return phase; }

    // Print a representation of the constraint to the file
    void show(FILE *out);

    // Get ID for BDD representation of constraint
    int get_nameid() { return tbdd_nameid(validation); }


    // Generate an Xor constraint as the sum of two constraints
    friend xor_constraint *xor_plus(xor_constraint *arg1, xor_constraint *arg2);
    // Compute the sum of a list of Xors.  Used by the xor_set class
    friend xor_constraint *xor_sum_list(xor_constraint **xlist, int len, int maxvar);
};

// Representation of a set of Xor constraints
class xor_set {
 private:
    std::vector<xor_constraint *> xlist;
    int maxvar;

 public:
    xor_set() { maxvar = 0; }

    ~xor_set();

    // Add an xor constraint to the set.
    // The code makes a copy of the constraint, and so
    // it is up to the caller to delete the original one.
    void add(xor_constraint &con);

    // Extract the validated sum of the Xors.
    // All constraints in the set are deleted
    xor_constraint *sum();

    // Perform Gauss - Jordan elimination to set of equations
    // to reduce it to a set of equations over only the external variables
    // nset updated to contain the new equations
    // May be degenerate, infeasible, or with real equations
    void gauss_jordan(ilist external_variables, xor_set &nset);

    // Testing properties of reduced set
    bool is_degenerate();  // Does not impose any constraints on external variables
    bool is_infeasible();  // Has not solutions

    size_t size() { return xlist.size(); }

    void clear();
};
} /* Namespace trustbdd */

#endif /* PSEUDOBOOLEAN */
