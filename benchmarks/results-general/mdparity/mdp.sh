#!/bin/bash
N=32
K=$((N/4))
T=$((K))
T1=$((T-1))

for S in {10..14}
do
    make cnf N=$N K=$K T=$T SEED=$S
    make knrun N=$N K=$K T=$T SEED=$S
    make cnrun N=$N K=$K T=$T SEED=$S
    make tgnrun N=$N K=$K T=$T SEED=$S

    make cnf N=$N K=$K T=$T1 SEED=$S
    make knrun N=$N K=$K T=$T1 SEED=$S
    make cnrun N=$N K=$K T=$T1 SEED=$S
    make tgnrun N=$N K=$K T=$T1 SEED=$S
done
