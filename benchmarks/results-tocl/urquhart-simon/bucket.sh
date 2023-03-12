#!/bin/bash
BS=123456

for M in {3..4}
do
    for DS in {0..4}
    do
	S=$((BS+DS))
	make bdata M=$M SEED=$S
	make rdata M=$M SEED=$S
    done
done


for M5 in {1..7}
do
    M=$((M5*5))
    for DS in {0..4}
    do
	S=$((BS+DS))
	make bdata M=$M SEED=$S
	make rdata M=$M SEED=$S
    done
done


	
