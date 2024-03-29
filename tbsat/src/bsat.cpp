/*========================================================================
  Copyright (c) 2022 Randal E. Bryant, Carnegie Mellon University

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

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

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "signal.h"
#include <sys/time.h>
#include <string.h>
#include "clause.h"

#include "tbdd.h"

/* Global values */

/* Time limit for execution.  0 = no limit */
int timelimit = 0;

extern bool solve(FILE *cnf_file, int verblevel, bool binary, int max_solutions, unsigned seed);

// BDD-based SAT solver

void usage(char *name) {
    printf("Usage: %s [-h] [-b] [-v VERB] [-i FILE.cnf] [-o FILE.lrat(b)] [-p FILE.order] [-s FILE.schedule] [-T FILE.btrace] [-m SOLNS] [-t TLIM] [-c CLIM] [-r SEED]\n", name);
    printf("  -h               Print this message\n");
    printf("  -v VERB          Set verbosity level (0-3)\n");
    printf("  -i FILE.cnf      Specify input file (otherwise use standard input)\n");
    printf("  -m SOLNS         Generate up to specified number of solutions\n");
    printf("  -t TLIM          Set time limit for execution (seconds)\n");
    printf("  -c CLIM          Set limit on number of input+proof clauses\n");
    printf("  -r SEED          Set seed for RNG (used to break ties)\n");
    exit(0);
}

void sigalrmhandler(int /*sig*/) {
    printf("Timeout after %d seconds\n", timelimit);
    printf("Elapsed seconds: %d.00\n", timelimit);
    exit(1);
}

void set_timeout(int tlim) {
    timelimit = tlim;
    signal(SIGALRM, sigalrmhandler);
    alarm(tlim);
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
    bool binary = false;
    int c;
    int verb = 1;
    unsigned seed = 1;
    int max_solutions = 1;
    while ((c = getopt(argc, argv, "hbv:i:o:p:s:m:t:T:c:r:")) != -1) {
        char buf[2] = { (char) c, '\0' };
        switch (c) {
        case 'h':
            usage(argv[0]);
        case 'v':
            verb = atoi(optarg);
            break;
        case 'm':
            max_solutions = atoi(optarg);
            break;
        case 't':
            set_timeout(atoi(optarg));
            break;
        case 'r':
            seed = atoi(optarg);
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
    double start = tod();
    if (solve(cnf_file, verb, binary, max_solutions, seed)) {
        if (verb >= 1) {
            printf("c Elapsed seconds: %.3f\n", tod()-start);
        }
    }
    return 0;
}
