#!/bin/bash

for N in {8..13}
do
    make cnf N=$N
    make bdata N=$N 
    done
done


	
