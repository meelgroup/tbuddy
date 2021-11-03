#include <iostream>
#include "clause.h"

extern bool solve(FILE *cnf_file);

// BDD-based SAT solver

int main(int argc, char *argv[]) {
  FILE *cnf_file = stdin;
  if (argc >= 2) {
    if (strcmp(argv[1], "-h") == 0) {
      std::cerr << "Usage: " << argv[0] << " [infile.cnf]" << std::endl;
      exit(1);
    }
    cnf_file = fopen(argv[1], "r");
    if (cnf_file == NULL) {
      std::cerr << "Couldn't open file " << argv[1] << std::endl;
      exit(1);
    }
  }
  if (solve(cnf_file)) {
    std::cout << "Done" << std::endl;
  }
  return 0;
}
