#include "../include/pseudoboolean.h"
#include "../include/ilist.h"
#include "../include/prover.h"
#include "solvertypesmini.h"
#include <vector>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <string.h>


using namespace CMSat;
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

void add_xor_to_cnf(ilist vs, bool phase, FILE* fout)
{
    for(uint64_t i = 0; i < 1ULL<<vs[-1]; i++) {
        const uint32_t hamm = hamm_w(i);
        if ((hamm % 2) == phase) {
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
    ilist l1 = ilist_new(3);
    ilist_resize(l1, 3);
    l1[0] = 1;
    l1[1] = 2;
    l1[2] = 3;

    ilist l2 = ilist_new(3);
    ilist_resize(l2, 3);
    l2[0] = 1;
    l2[1] = 2;
    l2[2] = 4;

    ilist l3 = ilist_new(2);
    ilist_resize(l3, 2);
    l3[0] = 3;
    l3[1] = 4;

    auto x1 = xor_constraint(l1, 1);
    add_xor_to_cnf(l1, 1, cnf_out);

    auto x2 = xor_constraint(l2, 1);
    add_xor_to_cnf(l2, 1, cnf_out);

    auto x3 = xor_constraint(l3, 0);
    add_xor_to_cnf(l3, 0, cnf_out);

    auto xs = xor_plus(&x1, &x2);
    auto xs2 = xor_plus(xs, &x3);

    ilist out = ilist_new(0);
    ilist_resize(out, 0);
    assert_clause(out);
}

void test_2()
{
    ilist l1 = ilist_new(1);
    ilist_resize(l1, 1);
    l1[0] = 4;
    add_xor_to_cnf(l1, 0, cnf_out);

    ilist l2 = ilist_new(4);
    ilist_resize(l2, 4);
    l2[0] = 1;
    l2[1] = 2;
    l2[2] = 3;
    l2[3] = 4;
    auto x2 = xor_constraint(l2, 1);
    add_xor_to_cnf(l2, 1, cnf_out);

    ilist l3 = ilist_new(3);
    ilist_resize(l3, 3);
    l3[0] = 1;
    l3[1] = 2;
    l3[2] = 3;
    auto x3 = xor_constraint(l3, 1);
    add_xor_to_cnf(l3, 1, cnf_out);

    auto xs = xor_plus(&x2, &x3);
    ilist out = ilist_new(1);
    ilist_resize(out, 1);
    out[0] = 4;
    assert_clause(out);
    fprintf(drat_out, "4 0\n");
}

void test_3()
{
    ilist l2 = ilist_new(4);
    ilist_resize(l2, 4);
    l2[0] = 1;
    l2[1] = 2;
    l2[2] = 3;
    l2[3] = 4;
    auto x2 = xor_constraint(l2, 1);
    add_xor_to_cnf(l2, 1, cnf_out);

    ilist l3 = ilist_new(4);
    ilist_resize(l3, 4);
    l3[0] = 1;
    l3[1] = 2;
    l3[2] = 3;
    l3[2] = 5;
    auto x3 = xor_constraint(l3, 1);
    add_xor_to_cnf(l3, 1, cnf_out);

    auto xs = xor_plus(&x2, &x3);
    ilist out = ilist_new(2);
    ilist_resize(out, 2);
    out[0] = -4;
    out[1] = 5;
    assert_clause(out);
    fprintf(drat_out, "4 0\n");
}

int main(int agc, char** argv)
{
    cnf_out = fopen("input.cnf", "w");
    drat_out = fopen("out.drat", "w");
    prover_init(drat_out, 5, 0, NULL, 0);

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

    fclose(drat_out);
    fclose(cnf_out);

    return 0;
}
