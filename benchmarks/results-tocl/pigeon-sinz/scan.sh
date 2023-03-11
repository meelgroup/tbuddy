#!/bin/bash

D=1

for DCOUNT in {1..1}
do
    D=$((D*10)) 
    for O in {1..9}
    do
        N=$((D*O))
        make cnf N=$N
        make sdata N=$N 
    done
done

for DCOUNT in {1..1}
do
    D=$((D*10))
    for O in {1..2}
    do
        N=$((D*O))
        make cnf N=$N
        make sdata N=$N
    done
done

	

