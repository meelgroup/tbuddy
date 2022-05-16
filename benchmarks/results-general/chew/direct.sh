#!/bin/bash

TLIM=650

S=1

for HN in {5..20}
do
    N=$((HN*2))
    make cnf N=$N SEED=$S
    make ddata N=$N SEED=$S TLIM=$TLIM
done


	
