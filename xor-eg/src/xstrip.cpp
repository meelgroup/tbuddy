// Scalable benchmark that involves both xor and clausal reasoning.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>

#include "pseudoboolean.h"


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

// Create 4*k 3-argument Xors
// Such that variables 1 and n are covered by one Xor each
// while all other variables are covered by two
// Set phase of one xor to 0, the rest to 1
// Result will be equivalent to x_1 ^ x_n = 0
static void gen_xors(int k) {
  int vars[3];
  int v;
  for (v = 1; v < 6*k; v+= 3) {
    vars[0] = v; vars[1] = v+1; vars[2] = v+2;
    int tphase = v == 1 ? 0 : 1;
    gen_xor(vars, 3, tphase);
    vars[0] = v+3;
    gen_xor(vars, 3, 1);
  }
}
    
// Generate two linear chains of implications
// One yields x_1 --*--> x_n
// The other yields x_n --*--> x_1
// Use different intermediate variables for the two chains
// so that can't use equivalence reasoning
static void gen_binaries(int k) {
  int lits[2];
  int i;
  int n = 6*k+1;
  lits[0] = -1; lits[1] = n;
  gen_clause(lits, 2);
  lits[0] = 1; lits[1] = -n;
  gen_clause(lits, 2);
  return;

  for (int v = 1; v < n-1; v+= 2) {
    lits[0] = -v; lits[1] = v+2;
    gen_clause(lits, 2);
  }
  lits[0] = 1; lits[1] = -2;
  gen_clause(lits, 2);
  for (int v = 2; v < n-1; v+= 2) {
    lits[0] = v; lits[1] = -(v+2);
    gen_clause(lits, 2);
  }
  lits[0] = n-1; lits[1] = -n;
  gen_clause(lits, 2);
}

// Write CNF File
static void gen_cnf(char *froot, int k) {
  char fname[strlen(froot) + 5];
  sprintf(fname, "%s.cnf", froot);
  int n = 6*k+1;
  FILE *cnf_file = fopen(fname, "w");
  if (!cnf_file) {
    fprintf(stderr, "Couldn't open file '%s'\n", fname);
    exit(1);
  }
  fprintf(cnf_file, "p cnf %d %d\n", n, (int) clauses.size());
  for (int c = 0; c < clauses.size(); c++) {
    ilist clause = clauses[c];
    ilist_print(clause, cnf_file, " ");
    fprintf(cnf_file, " 0\n");
  }
  fclose(cnf_file);
  printf("File %s: %d variables, %d clauses\n", fname, n, (int) clauses.size());
}

// Generate DRAT proof
static void gen_drat_proof(char *froot, int k) {
  char fname[strlen(froot) + 6];
  sprintf(fname, "%s.drat", froot);
  FILE *proof_file = fopen(fname, "w");
  if (!proof_file) {
    fprintf(stderr, "Couldn't open file '%s'\n", fname);
    exit(1);
  }
  int n = 6*k+1;
  // When generating DRAT proofs, don't need to supply the input clauses
  // or how many there are.
  tbdd_init(proof_file, n, 0, NULL, false);
  tbdd_set_verbose(2); // So that comments are included in proof

  // Use Xor reasoning to infer constraint x_1 ^ x_n = 1
  xor_set xset;
  for (int x = 0; x < xor_variables.size(); x++) {
    xor_constraint *xc = new xor_constraint(xor_variables[x], xor_phases[x]);
    xset.add(xc);
  }
  xor_constraint *sum = xset.sum();
  tbdd validation = sum->get_validation();
  // Validate clauses [x_1, x_n] and [-x_1, -x_n]
  // drat-trim can make these inferences, but doing so 
  //  explicitly will make the program issue a warning if something is wrong.
  ilist cpos = ilist_new(2);
  ilist_fill2(cpos, 1, n);
  ilist cneg = ilist_new(2);
  ilist_fill2(cneg, -1, -n);
  tbdd_validate_clause(cpos, validation);
  tbdd_validate_clause(cneg, validation);
  // Assert unit clauses [x_1] and [-x_1]
  ilist upos = ilist_new(1);
  ilist_fill1(upos, 1);
  assert_clause(upos);
  ilist uneg = ilist_new(1);
  ilist_fill1(uneg, -1);
  assert_clause(uneg);
  // Assert empty clause
  ilist empty = ilist_new(0);
  assert_clause(empty);
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
  if (argc != 2 || argc == 2 && strcmp(argv[1], "-h") == 0) {
    printf("Usage: %s k\n", argv[0]);
    exit(0);
  }
  char froot[strlen(argv[1] + 10)];
  sprintf(froot, "xstrip-%s", argv[1]);
  int k = atoi(argv[1]);

  double start = tod();
  gen_xors(k);
  gen_binaries(k);
  gen_cnf(froot, k);
  gen_drat_proof(froot, k);
  printf("Elapsed seconds: %.2f\n", tod()-start);
  return 0;
}
