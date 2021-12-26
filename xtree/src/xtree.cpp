// Scalable benchmark that involves both xor and clausal reasoning.
// Compare left-associate and right-associate xor trees.  One has flipped phase
// Use ring of implications for top-level equivalence, so that it does not correspond to 2-variable XOR

// The trees are defined over inputs 1 .. n.
// The left associative tree uses variables n .. 2n-1, with the root at R1 = 2n-1
// The right associate tree uses variables 2n .. 3n-2, with the root at R2 = 3n-1
// Variables Y1 = 3n-1 and Y2 = 3n are used as part of the top-level ring.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>

#include "pseudoboolean.h"

#ifndef BINARY
#define BINARY 0
#endif

#ifndef VERBOSITY
#define VERBOSITY 1
#endif

using namespace trustbdd;


// $begin xtree-abbrev 
// Shorthand names for the 4 top-level nodes
#define R1(n) (2*(n)-1)
#define R2(n) (3*(n)-2)
#define Y1(n) (3*(n)-1)
#define Y2(n) (3*(n))
// $end xtree-abbrev

// All clauses
std::vector<ilist> clauses;

// $begin xtree-global
////// Global data //////
// The parameters for the parity constraints
std::vector<ilist> xor_variables;
std::vector<int> xor_phases;
// $end xtree-global


static void gen_clause(int *lits, int len) {
    ilist clause = ilist_copy_list(lits, len);
    clauses.push_back(clause);
}

// For generating xor's: does word have odd parity?
static int parity(int w) {
    int odd = 0;
    while (w > 0) {
	odd ^= w & 0x1;
	w >>= 1;
    }
    return odd;
}

// Generate random permutation of numbers 1 .. n
static void rperm(int *dest, int n) {
    int i;
    for (i = 0; i < n; i++)
	dest[i] = i+1;
    for (i = 0; i < n-1; i++) {
	int index = random() % (n-i);
	int t = dest[i];
	dest[i] = dest[i+index];
	dest[i+index] = t;
    }
    if (VERBOSITY >= 2) {
	printf("Permutation:");
	for (i = 0; i < n; i++) {
	    printf(" %d", dest[i]);
	}
	printf("\n");
    }
}


// Generate CNF for arbitrary-input xor
static void gen_xor(int *vars, int len, int phase) {
    int bits;
    int elen = 1 << len;
    int lits[len];
    for (bits = 0; bits < elen; bits++) {
	int i;
	if (parity(bits) != phase)
	    continue;
	for (i = 0; i < len; i++)
	    lits[i] = (bits >> i) & 0x1 ? vars[i] : -vars[i];
	gen_clause(lits, len);
    }
    ilist ivars = ilist_copy_list(vars, len);
    xor_variables.push_back(ivars);
    xor_phases.push_back(phase);
}

static void xor3(int v1, int v2, int v3, int phase) {
    int vars[3] = {v1, v2, v3};
    gen_xor(vars, 3, phase);
}

// Create xor tree with randomly permuted inputs
// Optionally flip phase of first xor
static void rxtree(int n, int dest, bool flip) {
    int vars[n];
    rperm(vars, n);
    int src = 0;
    // Left associative, but with permuted variables
    xor3(vars[src], vars[src+1], dest, flip ? 0 : 1);
    src += 2;
    while (src < n) {
	xor3(vars[src], dest, dest+1, 1);
	src ++;
	dest ++;
    }
}

static void gen_xors(int n) {
    rxtree(n, n+1, true);
    rxtree(n, 2*n, false);
}
    
// Generate cycle of four implications
static void gen_binaries(int n) {
    int vars[4] = { R1(n), Y1(n), R2(n), Y2(n) };
    int lits[2];
    lits[0] = -vars[0]; lits[1] = vars[1];
    gen_clause(lits, 2);
    lits[0] = -vars[1]; lits[1] = vars[2];  
    gen_clause(lits, 2);
    lits[0] = -vars[2]; lits[1] = vars[3];  
    gen_clause(lits, 2);
    lits[0] = -vars[3]; lits[1] = vars[0];  
    gen_clause(lits, 2);
}

// Write CNF File
static void gen_cnf(char *fname, int n) {
    int vcount = 3*n;
    FILE *cnf_file = fopen(fname, "w");
    if (!cnf_file) {
	fprintf(stderr, "Couldn't open file '%s'\n", fname);
	exit(1);
    }
    fprintf(cnf_file, "p cnf %d %d\n", vcount, (int) clauses.size());
    for (int c = 0; c < clauses.size(); c++) {
	ilist clause = clauses[c];
	ilist_print(clause, cnf_file, " ");
	fprintf(cnf_file, " 0\n");
    }
    fclose(cnf_file);
    printf("File %s: %d variables, %d clauses\n", fname, vcount, (int) clauses.size());
}

// $begin xtree-drat
void gen_drat_proof(char *fname, int n) {
    ilist lits = ilist_new(2); // For adding clauses directly to proof
// $end xtree-drat
#if BINARY
    FILE *proof_file = fopen(fname, "wb");
#else
// $begin xtree-drat
    FILE *proof_file = fopen(fname, "w");
// $end xtree-drat
#endif
// $begin xtree-drat    
    if (!proof_file) {
	std::cerr << "Couldn't open file " << fname << std::endl;
	exit(1);
    }
    int vcount = 3*n;  // Total number of variables
// $end xtree-drat
    // TBDD initialization
#if BINARY
    tbdd_init_drat_binary(proof_file, vcount);
    tbdd_set_verbose(VERBOSITY);
#else
// $begin xtree-drat
    tbdd_init_drat(proof_file, vcount);  ///line:initialize
// $end xtree-drat
    tbdd_set_verbose(VERBOSITY);
#endif
// $begin xtree-drat    
    // Use parity reasoning to infer constraint R1 ^ R2 = 1
    xor_set xset; ///line:xset:start
    for (int x = 0; x < xor_variables.size(); x++) {
	xor_constraint xc(xor_variables[x], xor_phases[x]);
	xset.add(xc);
    } ///line:xset:end
    // Form the sum of the constraints
    xor_constraint *sum = xset.sum(); ///line:xset:sum

    // Assert inequivalence of R1 and R2, as is implied by XOR sum
    assert_clause(ilist_fill2(lits, R1(n), R2(n)));  ///line:xor:start
    assert_clause(ilist_fill2(lits, -R1(n), -R2(n))); ///line:xor:end
    // Assert unit clause for R1
    assert_clause(ilist_fill1(lits, R1(n))); ///line:unit
    // Assert empty clause
    assert_clause(ilist_resize(lits, 0)); ///line:empty

    // Finish up
    tbdd_done();
    fclose(proof_file);
    ilist_free(lits);
    std::cout << "File " << fname << " written" << std::endl << std::endl;
}
// $end xtree-drat

double tod() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == 0)
	return (double) tv.tv_sec + 1e-6 * tv.tv_usec;
    else
	return 0.0;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3 || argc == 2 && strcmp(argv[1], "-h") == 0) {
	printf("Usage: %s N [SEED]\n", argv[0]);
	exit(0);
    }
    char *anum = argv[1];
    char fnamec[strlen(anum) + 10];
    char fnamed[strlen(anum) + 10];
    int n = atoi(anum);

    if (argc == 3) {
	int seed = atoi(argv[2]);
	srandom(seed);
    }

    double start = tod();
    gen_xors(n);
    gen_binaries(n);
    sprintf(fnamec, "xtree-%s.cnf", anum);
    gen_cnf(fnamec, n);
#if BINARY
    sprintf(fnamed, "xtree-%s.dratb", anum);
#else
    sprintf(fnamed, "xtree-%s.drat", anum);
#endif
    gen_drat_proof(fnamed, n);
    printf("Elapsed seconds: %.2f\n", tod()-start);
    return 0;
}
	
