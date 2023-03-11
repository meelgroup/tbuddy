#!/bin/bash

TLIM=1000

S=1

for N in {8..13}
do
    make cnf N=$N
    make ldata N=$N
done


	
