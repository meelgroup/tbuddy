#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "clause.h"

extern bool solve(FILE *cnf_file, bool bucket, int verblevel);

// BDD-based SAT solver

void usage(char *name) {
  printf("Usage: %s [-h] [-b] [-v VERB] [-i FILE.cnf]\n", name);
  printf("  -h      Print this message\n");
  printf("  -b      Use bucket elimination\n");
  printf("  -v VERB Set verbosity level (0-3)\n");
  printf("  -i      Specify input file (otherwise use standard input)\n");
  exit(0);
}


int main(int argc, char *argv[]) {
  FILE *cnf_file = stdin;
  bool bucket = false;
  int c;
  int verb = 1;
  while ((c = getopt(argc, argv, "hbv:i:")) != -1) {
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
    default:
      std::cerr << "Unknown option '" << buf << "'" << std::endl;
      usage(argv[0]);
    }
  }
  if (solve(cnf_file, bucket, verb)) {
    if (verb >= 1)
      std::cout << "Done" << std::endl;
  }
  return 0;
}
