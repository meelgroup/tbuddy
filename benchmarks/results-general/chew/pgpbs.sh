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

M=100

for O in {1..20}
do
    N=$((M*O))
    for T in {1..4}
    do
        SEED=$((S+T))
	make cnf N=$N SEED=$SEED
	make pgdata N=$N SEED=$SEED
    done
done
