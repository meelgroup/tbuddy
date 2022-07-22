#!/bin/bash

S=123456

##D=100
##for DCOUNT in {3..3}
##do
##    D=$((D*10)) 
##    for O in {1..4}
##    do
##        N=$((D*O))
##        make cnf N=$N SEED=$S
##        make pgdata N=$N SEED=$S
##    done
##done

B=1000
M=100

for O in {1..9}
do
    N=$((B+M*O))
    make cnf N=$N SEED=$S
    make pgdata N=$N SEED=$S
done
