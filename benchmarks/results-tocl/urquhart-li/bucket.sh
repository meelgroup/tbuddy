#!/bin/bash

for M5 in {1..7}
do
    M=$((M5*5))
    make rdata M=$M 
done


	
