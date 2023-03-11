#!/bin/bash

for N5 in {1..7}
do
    N = $((N5*5))
    make bdata N=$N 
done


	
