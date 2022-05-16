#!/bin/bash

BS=123456
TLIM=600

for HN in {5..25}
do
    N=$((HN*2))
    for DS in {0..2}
    do
	S=$((BS+DS))
        make cnf N=$N SEED=$S
        make kdata N=$N SEED=$S TLIM=$TLIM
    done
done


	
