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

/* Shorthand names for the 4 top-level nodes */
#define R1(n) (2*(n)-1)
#define R2(n) (3*(n)-2)
#define Y1(n) (3*(n)-1)
#define Y2(n) (3*(n))


// Global data

// All clauses
std::vector<ilist> clauses;

// The parameters of all Xors
std::vector<ilist> xor_variables;
std::vector<int> xor_phases;

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
  printf("Permutation:");
  for (i = 0; i < n; i++) {
    printf(" %d", dest[i]);
  }
  printf("\n");
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
  const char *op = phase == 0 ? "==" : "^";
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
static void gen_cnf(char *froot, int n) {
  char fname[strlen(froot) + 5];
  sprintf(fname, "%s.cnf", froot);
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

// Generate DRAT proof
static void gen_drat_proof(char *froot, int n) {
  char fname[strlen(froot) + 6];
  sprintf(fname, "%s.drat", froot);
  FILE *proof_file = fopen(fname, "w");
  if (!proof_file) {
    fprintf(stderr, "Couldn't open file '%s'\n", fname);
    exit(1);
  }
  int vcount = 3*n;
  // When generating DRAT proofs, don't need to supply the input clauses
  // or how many there are.
  tbdd_init(proof_file, vcount, 0, NULL, false);
  tbdd_set_verbose(2); // So that comments are included in proof

  // Use Xor reasoning to infer constraint x_1 ^ x_n = 1
  xor_set xset;
  for (int x = 0; x < xor_variables.size(); x++) {
    xor_constraint *xc = new xor_constraint(xor_variables[x], xor_phases[x]);
    xset.add(xc);
  }
  xor_constraint *sum = xset.sum();
  int cbuf[ILIST_OVHD+2];
  ilist clause = ilist_make(cbuf, 2);
  // Assert inequivalence, extracted from XOR sum
  assert_clause(ilist_fill2(clause, R1(n), R2(n)));
  assert_clause(ilist_fill2(clause, -R1(n), -R2(n)));
  // Assert contradictory unit clauses
  assert_clause(ilist_fill1(clause, R1(n)));
  assert_clause(ilist_fill1(clause, -R1(n)));
  // Assert empty clause
  assert_clause(ilist_resize(clause, 0));
  tbdd_done();
  fclose(proof_file);
  printf("File %s written\n\n", fname);
}

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
  char froot[strlen(argv[1] + 10)];
  sprintf(froot, "xtree-%s", argv[1]);
  int n = atoi(argv[1]);
  if (argc == 3) {
    int seed = atoi(argv[2]);
    srandom(seed);
  }

  double start = tod();
  gen_xors(n);
  gen_binaries(n);
  gen_cnf(froot, n);
  gen_drat_proof(froot, n);
  printf("Elapsed seconds: %.2f\n", tod()-start);
  return 0;
}
