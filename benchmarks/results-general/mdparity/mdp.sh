#!/bin/bash
N=20
M=$((5*N/2))

for S in {10..11}
do
    make tsrun N=$N M=$M SEED=$S
    make tprun N=$N M=$M SEED=$S
    make turun N=$N M=$M SEED=$S
done
