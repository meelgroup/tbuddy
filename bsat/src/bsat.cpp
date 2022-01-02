#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include "clause.h"

#include "tbdd.h"

extern bool solve(FILE *cnf_file, FILE *proof_file, FILE *sched_file, bool bucket, int verblevel, proof_type_t ptype, bool binary);

// BDD-based SAT solver

void usage(char *name) {
    printf("Usage: %s [-h] [-b] [-v VERB] [-i FILE.cnf] [-o FILE.lrat(b)] [-s FILE.sched]\n", name);
    printf("  -h               Print this message\n");
    printf("  -b               Use bucket elimination\n");
    printf("  -v VERB          Set verbosity level (0-3)\n");
    printf("  -i FILE.cnf      Specify input file (otherwise use standard input)\n");
    printf("  -o FILE.lrat(b)  Specify output proof file (otherwise no proof)\n");
    printf("  -s FILE.sched    Specify schedule file\n");
    exit(0);
}


double tod() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == 0)
	return (double) tv.tv_sec + 1e-6 * tv.tv_usec;
    else
	return 0.0;
}

char *get_extension(char *name) {
    /* Look for '.' */
    for (int i = strlen(name)-1; i > 0; i--) {
	if (name[i] == '.')
	    return name+i+1;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *cnf_file = stdin;
    FILE *sched_file = NULL;
    FILE *proof_file = NULL;
    bool bucket = false;
    proof_type_t ptype = PROOF_NONE;
    bool binary = false;
    int c;
    int verb = 1;
    while ((c = getopt(argc, argv, "hbv:i:o:s:")) != -1) {
	char buf[2] = { (char) c, '\0' };
	char *extension;
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
	    extension = get_extension(optarg);
	    if (!extension) {
		std::cerr << "Unknown file type '" << optarg << "'" << std::endl;
		usage(argv[0]);
	    }
	    if (strcmp(extension, "drat") == 0) {
		binary = false;
		ptype = PROOF_DRAT;
	    } else if (strcmp(extension, "dratb") == 0) {
		binary = true;
		ptype = PROOF_DRAT;
	    } else if (strcmp(extension, "lrat") == 0) {
		binary = false;
		ptype = PROOF_LRAT;
	    } else if (strcmp(extension, "lratb") == 0) {
		binary = true;
		ptype = PROOF_LRAT;
	    } else {
		std::cerr << "Unknown file type '" << optarg << "'" << std::endl;
		usage(argv[0]);
	    }
	    break;
	default:
	    std::cerr << "Unknown option '" << buf << "'" << std::endl;
	    usage(argv[0]);
	}
    }
    double start = tod();
    if (solve(cnf_file, proof_file, sched_file, bucket, verb, ptype, binary)) {
	if (verb >= 1) {
	    printf("Elapsed seconds: %.2f\n", tod()-start);
	}
    }
    if (proof_file != NULL)
	fclose(proof_file);
    if (sched_file != NULL)
	fclose(sched_file);
    return 0;
}
