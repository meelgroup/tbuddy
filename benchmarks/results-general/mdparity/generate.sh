#!/bin/bash

rm -rf lpn-files
mkdir lpn-files

for N in {28,32,36}
do
  K=$((N/4))
  T=$((K))
  T1=$((T-1))
  for S in {10..14}
  do
    make cnf N=$N K=$K T=$T SEED=$S
    mv mdparity-n$N-k$K-t$T-s$S.cnf lpn-files/lpn-$N-$S-sat.cnf
    make cnf N=$N K=$K T=$T1 SEED=$S
    mv mdparity-n$N-k$K-t$T1-s$S.cnf lpn-files/lpn-$N-$S-unsat.cnf
  done
done
