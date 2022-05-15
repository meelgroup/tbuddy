#!/bin/bash

S=1

for HN in {5..6}
do
    N=$((HN*2))
    make cnf N=$N SEED=$S
    make ldata N=$N SEED=$S
done


	
