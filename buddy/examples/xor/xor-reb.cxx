#include "../include/pseudoboolean.h"
// REB: Don't need these
//#include "../include/ilist.h"
//#include "../include/prover.h"

// REB: I don't have a copy of this file
//#include "solvertypesmini.h"
#include <vector>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <string.h>


//using namespace CMSat;
using std::vector;
using std::cout;
using std::endl;

uint32_t hamm_w(const uint64_t x) {
    uint32_t w = 0;
    for(uint32_t i = 0;i < 64; i++) {
        w += ((x>>i)&1);
    }
    return w;
}

void add_header_to_cnf(FILE *fout, int vcount, int ccount)
{
    fprintf(fout, "p cnf %d %d\n", vcount, ccount);
}

void add_xor_to_cnf(ilist vs, bool phase, FILE* fout)
{
    for(uint64_t i = 0; i < 1ULL<<vs[-1]; i++) {
        const uint32_t hamm = hamm_w(i);
	// REB: Changed test
        if ((hamm % 2) != phase) {
            for(uint32_t i2 = 0; i2 < vs[-1]; i2++) {
                if (((i>>i2)&1) == 0) {
                    fprintf(fout, "%d ", vs[i2]);
                } else {
                    fprintf(fout, "%d ", -vs[i2]);
                }
            }
            fprintf(fout, "0\n");
        }
    }
}

FILE* cnf_out;
FILE* drat_out;

void test_1()
{
    // Need header
    add_header_to_cnf(cnf_out, 5, 2 * 4 + 1 * 2);
    // REB: Simplified
    ilist l1 = ilist_new(3);
    ilist_fill3(l1, 1, 2, 3);

    ilist l2 = ilist_new(3);
    ilist_fill3(l2, 1, 2, 4);

    ilist l3 = ilist_new(2);
    ilist_fill2(l3, 3, 4);

    // REB: Use xor set
    xor_set xset;

    // REB: Dynamically allocate
    // REB: Phases should be 1
    xset.add(new xor_constraint(l1, 1));
    add_xor_to_cnf(l1, 1, cnf_out);

    xset.add(new xor_constraint(l2, 1));
    add_xor_to_cnf(l2, 1, cnf_out);

    xset.add(new xor_constraint(l3, 1));
    add_xor_to_cnf(l3, 1, cnf_out);

    xor_constraint *xs = xset.sum();

    ilist out = ilist_new(0);
    assert_clause(out);
}

void test_2()
{
    // Need header
    add_header_to_cnf(cnf_out, 4, 1 + 8 + 4);

    xor_set xset;

    ilist l1 = ilist_new(1);
    ilist_fill1(l1, 4);
    add_xor_to_cnf(l1, 1, cnf_out);

    ilist l2 = ilist_new(4);
    ilist_fill4(l2, 1, 2, 3, 4);

    ilist l3 = ilist_new(3);
    ilist_fill3(l3, 1, 2, 3);

    xset.add(new xor_constraint(l2, 0));
    add_xor_to_cnf(l2, 0, cnf_out);

    xset.add(new xor_constraint(l3, 0));
    add_xor_to_cnf(l3, 0, cnf_out);
    
    xor_constraint *xs = xset.sum();

    // This assertion is not needed
    //    ilist out = ilist_new(1);
    //    ilist_fill1(out, -4);
    //    assert_clause(out);

    ilist empty = ilist_new(0);
    assert_clause(empty);
}

void test_3()
{
    add_header_to_cnf(cnf_out, 5, 2 + 8 + 8);

    xor_set xset;

    ilist u1 = ilist_new(1);
    ilist_fill1(u1, 4);
    add_xor_to_cnf(u1, 0, cnf_out);

    ilist u2 = ilist_new(1);
    ilist_fill1(u2, 5);
    add_xor_to_cnf(u2, 1, cnf_out);

    ilist l1 = ilist_new(4);
    ilist_fill4(l1, 1, 2, 3, 4);
    
    ilist l2 = ilist_new(4);
    ilist_fill4(l2, 1, 2, 3, 5);

    xset.add(new xor_constraint(l1, 1));
    add_xor_to_cnf(l1, 1, cnf_out);

    xset.add(new xor_constraint(l2, 1));
    add_xor_to_cnf(l2, 1, cnf_out);

    xor_constraint *xs = xset.sum();
    ilist empty = ilist_new(0);

    assert_clause(empty);

}

int main(int agc, char** argv)
{
    cnf_out = fopen("input.cnf", "w");
    drat_out = fopen("out.drat", "w");
    int vcount = 5;
    tbdd_init_drat(drat_out, vcount);
    // REB: This will cause DRAT file to have useful comments
    tbdd_set_verbose(2);


    if (agc != 2) {
        cout << "ERROR: You must give exactly one parameter, the test number" << endl;
        exit(-1);
    }
    if (strcmp(argv[1], "1") == 0) {
        test_1();
    } else if (strcmp(argv[1], "2") == 0) {
        test_2();
    } else if (strcmp(argv[1], "3") == 0) {
        test_3();
    } else {
        cout << "ERROR: unknown test!" << endl;
        exit(-1);
    }

    // REB: This shuts down more stuff and prints information
    tbdd_done();
    fclose(drat_out);
    fclose(cnf_out);


    return 0;
}
