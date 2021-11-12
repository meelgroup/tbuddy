#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "clause.h"

extern bool solve(FILE *cnf_file, FILE *proof_file, FILE *sched_file, bool bucket, int verblevel);

// BDD-based SAT solver

void usage(char *name) {
  printf("Usage: %s [-h] [-b] [-v VERB] [-i FILE.cnf] [-o FILE.lrat] [-s FILE.sched]\n", name);
  printf("  -h            Print this message\n");
  printf("  -b            Use bucket elimination\n");
  printf("  -v VERB       Set verbosity level (0-3)\n");
  printf("  -i FILE.cnf   Specify input file (otherwise use standard input)\n");
  printf("  -o FILE.lrat  Specify output proof file (otherwise use standard output)\n");
  printf("  -s FILE.sched Specify schedule file\n");
  exit(0);
}


double tod() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == 0)
    return (double) tv.tv_sec + 1e-6 * tv.tv_usec;
  else
    return 0.0;
}

int main(int argc, char *argv[]) {
  FILE *cnf_file = stdin;
  FILE *sched_file = NULL;
  FILE *proof_file = stdout;
  bool bucket = false;
  int c;
  int verb = 1;
  while ((c = getopt(argc, argv, "hbv:i:o:s:")) != -1) {
    char buf[2] = { c, 0 };
    switch (c) {
    case 'h':
      usage(argv[0]);
    case 'b':
      bucket = true;
      break;
    case 'v':
      verb = atoi(optarg);
      break;
    case 'i':
      cnf_file = fopen(optarg, "r");
      if (cnf_file == NULL) {
	std::cerr << "Couldn't open file " << optarg << std::endl;
	exit(1);
      }
      break;
    case 's':
      sched_file = fopen(optarg, "r");
      if (sched_file == NULL) {
	std::cerr << "Couldn't open file " << optarg << std::endl;
	exit(1);
      }
      break;
    case 'o':
      proof_file = fopen(optarg, "w");
      if (proof_file == NULL) {
	std::cerr << "Couldn't open file " << optarg << std::endl;
	exit(1);
      }
      break;
    default:
      std::cerr << "Unknown option '" << buf << "'" << std::endl;
      usage(argv[0]);
    }
  }
  double start = tod();
  if (solve(cnf_file, proof_file, sched_file, bucket, verb)) {
    if (verb >= 1) {
      printf("Elapsed time = %.2f seconds\n", tod()-start);
    }
      
  }
  return 0;
}
