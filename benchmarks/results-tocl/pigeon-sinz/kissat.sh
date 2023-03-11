#!/bin/bash

for N in {8..13}
do
    make cnf N=$N
    make kdata N=$N 
done


	
