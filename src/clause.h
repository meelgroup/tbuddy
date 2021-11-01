#pragma once

#include <vector>
#include <cstdint>
#include <stdio.h>
#include <fstream>

// Representations of clauses and sets of clauses

// Clause is a vector of literals, each of which is a negative or positive integer.
// Tautologies are detected and represented as clause of the form -x, x
// Clauses put in canonical ordering with duplicate literals removed, tautologies detected,
// and literals ordered in descending order of the variables
class Clause {
 private:
    std::vector<int32_t> contents;
    bool is_tautology;
 public:

    Clause();

    Clause(int32_t *array, size_t len);

    Clause(FILE *infile);

    void add(int32_t val);

    size_t length();

    bool tautology();

    int32_t max_variable();

    void canonize();

    void show();

    void show(FILE *outfile);

    void show(std::ofstream &outstream);

};

// CNF is a collection of clauses.  Can read from a DIMACS format CNF file
class CNF {
 private:
    std::vector<Clause *> clauses;
    int32_t maxVar;
    bool read_failed;

 public:
    CNF();

    // Read clauses DIMACS format CNF file
    CNF(FILE *infile);
    // Did last read fail?
    bool failed();

    // Add a new clause
    void add(Clause *clp);

    // Generate DIMACS CNF representation to stdout, outfile, or outstream
    void show();
    void show(FILE *outfile);
    void show(std::ofstream &outstream);

    // Return number of clauses
    size_t clause_count();
    // Return ID of maximum variable encountered
    int32_t max_variable();
};
