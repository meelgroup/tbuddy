#include "../include/pseudoboolean.h"
#include "../include/ilist.h"
#include "../include/prover.h"
#include "solvertypesmini.h"
#include <vector>
#include <math.h>
#include <stdio.h>
#include <iostream>


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

void add_xor_to_cnf(ilist vs, bool out, FILE* fout)
{
    cout << "sz: " << vs[-1] << endl;
    for(uint64_t i = 0; i < 1ULL<<vs[-1]; i++) {
        const uint32_t hamm = hamm_w(i);
        cout << "Hamm w: " << hamm << endl;
        if ((hamm % 2) == out) {
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

int main()
{
    FILE* cnf_out = fopen("input.cnf", "w");
    FILE* drat_out = fopen("out.drat", "w");
    prover_init(drat_out, 4, 8, NULL, 0);


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

    auto x1 = xor_constraint(l1, (int)1);
    add_xor_to_cnf(l1, 1, cnf_out);

    auto x2 = xor_constraint(l2, (int)1);
    add_xor_to_cnf(l2, 1, cnf_out);

    auto x3 = xor_constraint(l3, (int)0);
    add_xor_to_cnf(l3, 0, cnf_out);

    xor_constraint *xs = xor_plus(&x1, &x2);
    xor_constraint *xs2 = xor_plus(xs, &x3);

    ilist out = ilist_new(0);
    assert_clause(out);

    fclose(drat_out);
    fclose(cnf_out);

    return 0;
}
