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
#include "prover.h"

using namespace trustbdd;

void usage(const char *name) {
    printf("Usage: %s [-h] [-c] [-g] -n N [-v VLEVEL] [-m (d|f|n)] [-b] [-s SEED] [-r ROOT]\n", name);
    printf("  -h         Print this information\n");
    printf("  -g         Use Gaussian elimination\n");
    printf("  -n  N      Set number of problem variables\n");
    printf("  -v VLEVEL  Set verbosity level\n");
    printf("  -m (d|f|n)   Set proof type (d=DRAT, f=FRAT, n=No proof)\n");
    printf("  -n         Use binary files\n");
    printf("  -s SEED    Set random seed\n");
    printf("  -r ROOT    Root of CNF and proof files\n");
    exit(0);
}

// $begin xtree-abbrev 
// Shorthand names for the 4 top-level nodes
#define R1(n) (2*(n)-1)
#define R2(n) (3*(n)-2)
#define Y1(n) (3*(n)-1)
#define Y2(n) (3*(n))
// $end xtree-abbrev

// Input clauses
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
void gen_drat_proof(char *fname, int n, int vlevel) {
    ilist lits = ilist_new(2); // For adding clauses directly to proof
    FILE *proof_file = fopen(fname, "w");
    if (!proof_file) {
	std::cerr << "Couldn't open file " << fname << std::endl;
	exit(1);
    }
    int vcount = 3*n;  // Total number of variables
    // TBDD initialization
    tbdd_set_verbose(vlevel);
    tbdd_init_drat(proof_file, vcount);  ///line:drat:initialize
    // Use parity reasoning to infer constraint R1 ^ R2 = 1
    xor_set xset; ///line:drat:xset:start
    for (int x = 0; x < xor_variables.size(); x++) {
	xor_constraint xc(xor_variables[x], xor_phases[x]);
	xset.add(xc);
    } ///line:drat:xset:end
    // Form the sum of the constraints
    xor_constraint *sum = xset.sum(); ///line:drat:xset:sum
    // Assert inequivalence of R1 and R2, as is implied by XOR sum
    assert_clause(ilist_fill2(lits, R1(n), R2(n)));  ///line:drat:xor:start
    assert_clause(ilist_fill2(lits, -R1(n), -R2(n))); ///line:drat:xor:end
    // Assert unit clause for R1
    assert_clause(ilist_fill1(lits, R1(n))); ///line:drat:unit
    // Assert empty clause
    assert_clause(ilist_resize(lits, 0)); ///line:drat:empty
    // Finish up
    delete sum; // Free underlying BDDs.  Delete clauses
    tbdd_done();
    fclose(proof_file);
    ilist_free(lits);
    std::cout << "File " << fname << " written" << std::endl << std::endl;
}
// $end xtree-drat

// $begin xtree-drat-gauss
void gen_drat_gauss_proof(char *fname, int n, int vlevel) {
    ilist lits = ilist_new(2); // For adding clauses directly to proof
    ilist externals = ilist_new(2);
    FILE *proof_file = fopen(fname, "w");
    if (!proof_file) {
	std::cerr << "Couldn't open file " << fname << std::endl;
	exit(1);
    }
    int vcount = 3*n;  // Total number of variables
    // TBDD initialization
    tbdd_set_verbose(vlevel);
    tbdd_init_drat(proof_file, vcount);  ///line:drat-gauss:initialize
    // Use parity reasoning to infer constraint R1 ^ R2 = 1
    xor_set xset; ///line:drat-gauss:xset:start
    for (int x = 0; x < xor_variables.size(); x++) {
	xor_constraint xc(xor_variables[x], xor_phases[x]);
	xset.add(xc);
    } ///line:drat-gauss:xset:end
    ilist_fill2(externals, R1(n), R2(n));
    xor_set reduced;
    xset.gauss_jordan(externals, reduced); ///line:drat-gauss:xset:gauss
    // Assert inequivalence of R1 and R2, as is implied by XOR sum
    assert_clause(ilist_fill2(lits, R1(n), R2(n)));  ///line:drat-gauss:xor:start
    assert_clause(ilist_fill2(lits, -R1(n), -R2(n))); ///line:drat-gauss:xor:end
    // Assert unit clause for R1
    assert_clause(ilist_fill1(lits, R1(n))); ///line:drat-gauss:unit
    // Assert empty clause
    assert_clause(ilist_resize(lits, 0)); ///line:drat-gauss:empty
    // Finish up
    reduced.clear(); // Free underlying BDDs.  Delete clauses
    tbdd_done();
    fclose(proof_file);
    ilist_free(lits);
    ilist_free(externals);
    std::cout << "File " << fname << " written" << std::endl << std::endl;
}
// $end xtree-drat-gauss


void gen_dratb_proof(char *fname, int n, int vlevel) {
    ilist lits = ilist_new(2); // For adding clauses directly to proof
    FILE *proof_file = fopen(fname, "wb");
    if (!proof_file) {
	std::cerr << "Couldn't open file " << fname << std::endl;
	exit(1);
    }
    int vcount = 3*n;  // Total number of variables
    // TBDD initialization
    tbdd_set_verbose(vlevel);
    tbdd_init_drat_binary(proof_file, vcount);
    // Use parity reasoning to infer constraint R1 ^ R2 = 1
    xor_set xset; ///line:dratb:xset:start
    for (int x = 0; x < xor_variables.size(); x++) {
	xor_constraint xc(xor_variables[x], xor_phases[x]);
	xset.add(xc);
    } ///line:dratb:xset:end
    // Form the sum of the constraints
    xor_constraint *sum = xset.sum(); ///line:dratb:xset:sum

    // Assert inequivalence of R1 and R2, as is implied by XOR sum
    assert_clause(ilist_fill2(lits, R1(n), R2(n)));  ///line:dratb:xor:start
    assert_clause(ilist_fill2(lits, -R1(n), -R2(n))); ///line:dratb:xor:end
    // Assert unit clause for R1
    assert_clause(ilist_fill1(lits, R1(n))); ///line:dratb:unit
    // Assert empty clause
    assert_clause(ilist_resize(lits, 0)); ///line:dratb:empty
    // Finish up
    delete sum; // Free underlying BDDs.  Delete clauses
    tbdd_done();
    fclose(proof_file);
    ilist_free(lits);
    std::cout << "File " << fname << " written" << std::endl << std::endl;
}

// $begin xtree-frat
void gen_frat_proof(char *fname, int n, int vlevel) {
    ilist lits = ilist_new(2); // For adding clauses directly to proof
    ilist dels = ilist_new(3); // For deleting intermediate clauses
    FILE *proof_file = fopen(fname, "w");
    if (!proof_file) {
	std::cerr << "Couldn't open file " << fname << std::endl;
	exit(1);
    }
    int vcount = 3*n;  // Tracks number of variables
    int ccount = clauses.size();    // Tracks number of clauses
    // Initialization
    // FRAT file requires declaration of input clauses
    for (int cid = 1; cid <= clauses.size(); cid++)  ///line:frat:initialize:start
	insert_frat_clause(proof_file, 'o', cid, clauses[cid-1], false);
    tbdd_set_verbose(vlevel);
    tbdd_init_frat(proof_file, &vcount, &ccount);  ///line:frat:initialize:end
    // Use parity reasoning to infer constraint R1 ^ R2 = 1
    xor_set xset; ///line:frat:xset:start
    for (int x = 0; x < xor_variables.size(); x++) {
	xor_constraint xc(xor_variables[x], xor_phases[x]);
	xset.add(xc);
    } ///line:frat:xset:end
    // Form the sum of the constraints
    xor_constraint *sum = xset.sum(); ///line:frat:xset:sum

    // Assert inequivalence of R1 and R2, as is implied by XOR sum
    tbdd vd = sum->get_validation();   ///line:frat:xor:start
    int c1 = tbdd_validate_clause(ilist_fill2(lits, R1(n), R2(n)), vd);
    int c2 = tbdd_validate_clause(ilist_fill2(lits, -R1(n), -R2(n)), vd); ///line:frat:xor:end
    // Assert unit clause for R1
    int c3 = assert_clause(ilist_fill1(lits, R1(n))); ///line:frat:unit
    // Assert empty clause
    int c4 = assert_clause(ilist_resize(lits, 0)); ///line:frat:empty
    // Finish up
    delete_clauses(ilist_fill3(dels, c1, c2, c3)); ///line:frat:finish:start
    delete sum; 
    vd = tbdd_tautology();
    tbdd_done();  // Eliminates previous reference ///line:frat:finish:end
    // FRAT requires declaring all remaining clauses
    insert_frat_clause(proof_file, 'f', c4, ilist_resize(lits, 0), false); ///line:frat:finalize:start
    for (int cid = 1; cid <= clauses.size(); cid++)
	insert_frat_clause(proof_file, 'f', cid, clauses[cid-1], false);///line:frat:finalize:end
    fclose(proof_file);
    ilist_free(lits);
    ilist_free(dels);
    std::cout << "File " << fname << " written" << std::endl << std::endl;
}
// $end xtree-frat

double tod() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == 0)
	return (double) tv.tv_sec + 1e-6 * tv.tv_usec;
    else
	return 0.0;
}


int main(int argc, char *argv[]) {
    int vlevel = 1;
    int n = 0;
    proof_type_t ptype = PROOF_DRAT;
    bool do_binary = false;
    bool do_gauss = false;
    char *root = NULL;
    char rootbuf[1024];
    char fnamec[1024];
    char fnamep[1024];
    int seed = -1;

    int c;
    while ((c = getopt(argc, argv, "hgn:v:m:bs:r:")) != -1) {
	switch (c) {
	case 'h':
	    usage(argv[0]);
	    return 0;
	case 'g':
	    do_gauss = true;
	    break;
	case 'n':
	    n = atoi(optarg);
	    break;
	case 'v':
	    vlevel = atoi(optarg);
	    break;
	case 'm':
	    switch (optarg[0]) {
	    case 'd':
		ptype = PROOF_DRAT;
		break;
	    case 'f':
		ptype = PROOF_FRAT;
		break;
	    case 'n':
		ptype = PROOF_NONE;
		break;
	    default:
		printf("Unknown proof type '%c'\n", optarg[0]);
		usage(argv[0]);
		break;
	    }
	    break;
	case 'b':
	    do_binary = true;
	    break;
	case 's':
	    seed = atoi(optarg);
	    break;
	case 'r':
	    root = optarg;
	    break;
	}
    }
    if (n == 0) {
	printf("Must specify value of N\n");
	usage(argv[0]);
    }
    if (seed > 0)
	srandom(seed);
    if (!root) {
	if (seed > 0)
	    sprintf(rootbuf, "xtree-%d-%d", n, seed);
	else
	    sprintf(rootbuf, "xtree-%d", n);
	root = rootbuf;
    }
    sprintf(fnamec, "%s.cnf", root);
    char mchar = ptype == PROOF_DRAT ? 'd' : 'f';
    const char *suffix = do_binary ? "b" : "";
    sprintf(fnamep, "%s.%crat%s", root, mchar, suffix);
    double start = tod();
    gen_xors(n);
    gen_binaries(n);
    gen_cnf(fnamec, n);
    if (ptype == PROOF_DRAT) {
	if (do_binary)
	    gen_dratb_proof(fnamep, n, vlevel);
	else {
	    if (do_gauss)
		gen_drat_gauss_proof(fnamep, n, vlevel);
	    else
		gen_drat_proof(fnamep, n, vlevel);
	}
    } else if (ptype == PROOF_FRAT) {
	gen_frat_proof(fnamep, n, vlevel);
    }
    printf("Elapsed seconds: %.2f\n", tod()-start);
    return 0;
}
	
