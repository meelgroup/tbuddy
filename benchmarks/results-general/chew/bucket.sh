#!/bin/bash

S=1

D=1
for DCOUNT in {1..2}
do
    D=$((D*10)) 
    for O in {1..9}
    do
        N=$((D*O))
        make cnf N=$N SEED=$S
        make bdata N=$N SEED=$S
    done
done

for DCOUNT in {1..1}
do
    D=$((D*10))
    for O in {1..8}
    do
        N=$((D*O))
        make cnf N=$N SEED=$S
        make bdata N=$N SEED=$S
    done
done


	
