#!/bin/bash

for N10 in {1..9}
do
    N=$((N10*10)) 
    make spigeon N=$N 
done

for N25 in {4..10}
do
    N=$((N25*25)) 
    make spigeon N=$N 
done

	

