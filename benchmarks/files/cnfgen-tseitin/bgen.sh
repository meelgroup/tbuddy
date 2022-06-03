#!/bin/bash

SEED=1

W=5
D=1
for E in {0..2}

do
    D=$((D*10))
    for V in {1..9}
    do
	N=$((D*V))
	cnfgen --seed $SEED tseitin randomodd gnd $N $W > tseitin-$N-w$W-s$SEED.cnf
    done
done
	
D=$((D*10))
N=$D
cnfgen --seed $SEED tseitin randomodd gnd $N $W > tseitin-$N-w$W-s$SEED.cnf
