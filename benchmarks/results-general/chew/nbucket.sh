#!/bin/bash

S=123456

D=1
for DCOUNT in {1..4}
do
    D=$((D*10)) 
    for O in {1..9}
    do
        N=$((D*O))
        make cnf N=$N SEED=$S
        make nbdata N=$N SEED=$S
    done
done

B=10000
M=1000

for O in {1..3}
do
    N=$((B+M*O))
    make cnf N=$N SEED=$S
    make nbdata N=$N SEED=$S
done

make clean	