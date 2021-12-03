#!/bin/bash
set -e
set -x

./xor
./fix_cnf.py input.cnf input.cnf-fixed
./drat-trim input.cnf-fixed out.drat
