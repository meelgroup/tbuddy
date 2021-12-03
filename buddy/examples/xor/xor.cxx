#include "../include/pseudoboolean.h"
#include "solvertypesmini.h"

using namespace CMSat;

int main()
{
    ilist x = ilist_new(2);
    x[0] = 1;
    x[1] = -2;
    //auto v = validation();

    auto x1 = xor_constraint(x, (int)0);
    auto x2 = xor_constraint(x, (int)1);
    xor_constraint *xs = xor_plus(x1, x2);

    return 0;
}
