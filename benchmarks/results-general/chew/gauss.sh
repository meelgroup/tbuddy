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
        make gdata N=$N SEED=$S
	make clean
    done
done

for DCOUNT in {1..1}
do
    D=$((D*10))
    for O in {1..1}
    do
        N=$((D*O))
        make cnf N=$N SEED=$S
        make gdata N=$N SEED=$S
	make clean
    done
done

	