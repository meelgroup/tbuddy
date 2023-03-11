#!/bin/bash

TLIM=1000

S=1

for HN in {5..20}
do
    N=$((HN*2))
    make cnf N=$N SEED=$S
    make ldata N=$N SEED=$S TLIM=$TLIM
done


	
