#!/bin/bash


for HN in {5..20}
do
    N=$((HN*2))
    for S in {1..3}
    do
        make cnf N=$N SEED=$S
        make kdata N=$N SEED=$S
    done
done


	
