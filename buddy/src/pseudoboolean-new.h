/*========================================================================
  Copyright (c) 2022 Randal E. Bryant, Carnegie Mellon University
  
  This code was not included in the original BuDDy distribution and is
  therefore not subject to any of its licensing terms.

  Permission is hereby granted, free of
  charge, to any person obtaining a copy of this software and
  associated documentation files (the "Software"), to deal in the
  Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following
  conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
========================================================================*/


/* Interface for proof-generating operations on Pseudo-Boolean functions */
/* Initial version on supports XOR constraints */

#ifndef _PSEUDOBOOLEAN_H
#define _PSEUDOBOOLEAN_H

#include <vector>
#include <unordered_set>
#include <cstdint>
#include "tbdd.h"

namespace trustbdd{

// A handy class for generating random numbers with own seed
// Pseudo-random number generator based on the Lehmer MINSTD RNG
// Use to both randomize selection and to provide a unique value
// for each cost.

class Sequencer {

public:
    Sequencer(int s) { seed = s; }

    Sequencer(void) { seed = default_seed; }

    Sequencer(Sequencer &s) { seed = s.seed; }

    void set_seed(uint64_t s) { seed = s == 0 ? 1 : s; next() ; next(); }

    uint32_t next() { seed = (seed * mval) % groupsize; return seed; }

    // Return next pseudo random value in interval [0.0, 1.0)
    double pseudo_double() { uint32_t val = next(); return (double) val / (double) groupsize; }

    // Return next pseudo random value in range [0, m)
    int pseudo_int(int m) { return (int) (m * pseudo_double()); }

private:
    uint64_t seed;
    const uint64_t mval = 48271;
    const uint64_t groupsize = 2147483647LL;
    const uint64_t default_seed = 123456;

};


// An Xor constraint is represented by a set of variables and a phase
// It also contains a TBDD validation 
class xor_constraint {
 private:
    ilist variables;
    int phase;
    tbdd validation;
    // How many clauses were added to justify this clause?
    int generated_clause_count;

 public:
    // Empty constraint represents a tautology
    xor_constraint() { variables = ilist_new(0); phase = 0; validation = trustbdd::tbdd_tautology(); }

    // Construct Xor constraint extracted from product of clauses
    // vfun indicates the TBDD representation of that product
    // For use when generating LRAT proofs
    // The list of variables is used within the constraint and deleted by the Xor constraint destructor
    xor_constraint(ilist vars, int p, trustbdd::tbdd &vfun);

    // Construct Xor constraint validated by product of two tbdds
    // vfun1, vfun2 indicates TBDD representations of validations
    // For use when generating LRAT proofs
    // The list of variables is used within the constraint and deleted by the Xor constraint destructor
    xor_constraint(ilist vars, int p, trustbdd::tbdd &vfun1, trustbdd::tbdd &vfun2);


    // Assert that Xor constraint is implied by the clauses
    // For use when generating DRAT proofs
    xor_constraint(ilist vars, int p);

    // Copy an Xor constraint, duplicating the data
    xor_constraint(xor_constraint &x) { variables = ilist_copy(x.variables); phase = x.phase; validation = x.validation; }
    
    // Destructor deletes the list of variables
    ~xor_constraint(void) { ilist_free(variables); variables = NULL; validation = trustbdd::tbdd_null();  }

    // Does the constraint have NO solutions?
    bool is_infeasible(void) { return ilist_length(variables) == 0 && phase != 0; }

    // Does the constraint impose any restrictions on any variables?
    bool is_degenerate(void) { return ilist_length(variables) == 0 && phase == 0; }
    
    // Use xor constraint to validate a clause
    int validate_clause(ilist clause);

    // Get the validation TBDD
    tbdd get_validation() { return validation; }

    ilist get_variables() { return variables; }

    int get_phase() { return phase; }

    int get_length() { return ilist_length(variables); }

    // Print a representation of the constraint to the file
    void show(FILE *out);

    // Get ID for BDD representation of constraint
    int get_nameid() { return tbdd_nameid(validation); }

    // How many clauses were added to create/justify this clause?
    int get_clause_count() { return generated_clause_count; }

    // Generate an Xor constraint as the sum of two constraints
    friend xor_constraint *xor_plus(xor_constraint *arg1, xor_constraint *arg2);
    // Compute the sum of a list of Xors.  Used by the xor_set class
    friend xor_constraint *xor_sum_list(xor_constraint **xlist, int len, int maxvar, unsigned seed);
};

// Generate an Xor constraint as the sum of two constraints
xor_constraint *xor_plus(xor_constraint *arg1, xor_constraint *arg2);
// Compute the sum of a list of Xors.  Used by the xor_set class
xor_constraint *xor_sum_list(xor_constraint **xlist, int len, int maxvar);


// Representation of a set of Xor constraints
class xor_set {
 private:
    int maxvar;
    unsigned seed;

 public:
    // Vector of constraints
    // Normally only read these
    std::vector<xor_constraint *> xlist;

    xor_set() { maxvar = 0; }

    ~xor_set();

    void set_seed(unsigned s) { seed = s; }

    // Add an xor constraint to the set.
    // The code makes a copy of the constraint, and so
    // it is up to the caller to delete the original one.
    void add(xor_constraint &con);

    // Extract the validated sum of the Xors.
    // All constraints in the set are deleted
    xor_constraint *sum();

    // Perform Gauss - Jordan elimination on set of equations
    // to reduce it to two sets of equations:
    //    iset over internal + external variables with internal pivots
    //       This set is only needed when generating multiple solutions
    //    eset over external variables with external pivots
    //       This set can be used to determine whether the formula is sat/unsat
    //       It is put in Jordan from
    // Return list of pivots in elimination order
    ilist gauss_jordan(std::unordered_set<int> &internal_variables, xor_set &eset, xor_set &iset);

    // Testing properties of eset
    bool is_infeasible();  // Has no solution

    size_t size() { return xlist.size(); }

    void clear();

};

    ///////////////////////////////////////////////////////////////////////////////
    //  Pseudo-Boolean (PB) constraints
    ///////////////////////////////////////////////////////////////////////////////

    typedef enum { PB_EQ, PB_LE, PB_GE, PB_LT, PB_GT } pb_relation_t;

// An (normalized) PB constraint is represented by:
//   1. A list of variables
//   2. A list of coefficients
//   3. A relation operator (PB_EQ or PB_LE)
//   4. A constant value
//   4. A modulus (0 for non-modular arithmetic)
//   5. A TBDD validation (set to tautology when working with unvalidated constraints)
class pb_constraint {
 private:
    ilist variables;
    ilist coefficients;
    pb_relation_t relation;
    int constant;
    int modulus;
    tbdd validation;

    // Private constructor:
    // Construct PB constraint validated by conjunction of two tbdds
    // vfun1, vfun2 indicates TBDD representations of validations
    // 
    pb_constraint(ilist lits, ilist coeffs, pb_relation_t rel, int constant, int modulus, trustbdd::tbdd &vfun1, trustbdd::tbdd &vfun2);


 public:
    // Empty constraint represents a tautology
    pb_constraint() { 
	variables = ilist_new(0);
	coefficients = ilist_new(0);
	relation = PB_EQ;
	constant = 0;
	modulus = 0;
	validation = trustbdd::tbdd_tautology(); 
    }

    // Construct PB representation, validated by TBDD representation
    // of underlying Boolean function.
    // Allow general form and normalize
    pb_constraint(ilist lits, ilist coeffs, pb_relation_t rel, int constant, int modulus, trustbdd::tbdd &vfun);

    // Copy an PB constraint, duplicating the data
    pb_constraint(pb_constraint &x);
    
    // Destructor deletes the list of variables
    ~pb_constraint(void);

    // Does the constraint have NO solutions?
    bool is_infeasible(void);

    // Does the constraint impose any restrictions on any variables?
    bool is_degenerate(void);
    
    // Use pb constraint to validate a clause
    int validate_clause(ilist clause);

    // Get the validation TBDD
    tbdd get_validation() { return validation; }

    ilist get_variables() { return variables; }

    ilist get_coefficients() { return coefficients; }

    pb_relation_t get_relation() { return relation; }

    int get_constant() { return constant; }

    int get_modulus() { return modulus; }

    int get_length() { return ilist_length(variables); }

    // Print a representation of the constraint to the file
    void show(FILE *out);

    // Get ID for BDD representation of constraint
    int get_nameid() { return tbdd_nameid(validation); }

    // How many clauses were added to create/justify this clause?
    int get_clause_count() { return generated_clause_count; }

    // Generate an PB constraint as the sum of two constraints
    friend pb_constraint *pb_plus(pb_constraint *arg1, pb_constraint *arg2);

};

// Generate an PB constraint as the sum of two constraints
pb_constraint *pb_plus(pb_constraint *arg1, pb_constraint *arg2);
    
} /* Namespace trustbdd */

#endif /* PSEUDOBOOLEAN */
