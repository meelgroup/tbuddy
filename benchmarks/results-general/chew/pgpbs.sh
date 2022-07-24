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

M=200

B=2000
for O in {1..10}
do
    N=$((B+M*O))
    for T in {1..4}
    do
        SEED=$((S+T))
	make cnf N=$N SEED=$SEED
	make pgdata N=$N SEED=$SEED
    done
done
