#!/bin/bash

BS=123456

for N in {3..4}
do
    for DS in {0..4}
    do
	S=$((BS+DS))
        make kdata N=$N SEED=$S
    done
done


	
