#!/bin/bash

S=123456

##D=1
##for DCOUNT in {1..2}
##do
##    D=$((D*10)) 
##    for O in {1..9}
##    do
##        N=$((D*O))
##        make cnf N=$N SEED=$S
##        make pbdata N=$N SEED=$S
##    done
##done

B=2000
M=100

for O in {3..7}
do
    N=$((B+M*O*2))
    make cnf N=$N SEED=$S
    make pbdata N=$N SEED=$S
done
