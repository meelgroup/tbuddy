#!/bin/bash
set -e
set -x

echo "---> Running ./xor $1"
./xor $1
echo "---> Running CNF header generation"
./fix_cnf.py input.cnf input.cnf-fixed
echo "---> Running DRAT-trim"
./drat-trim input.cnf-fixed out.drat
