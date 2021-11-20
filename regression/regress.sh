#!/bin/sh

make clean
rm -f *.ldata *.sdata *.bdata

make 01-unsat2.ldata
make 01-unsat2.bdata
make 02-xorImpliesOr3.ldata
make 02-xorImpliesOr3.bdata
make 03-chew-10.bdata
make 04-chess-32.sdata
make 05-pigeon-sinz-32.sdata
make 06-urquhart-li-09.bdata

echo "\nBelow, should get 8 lines of 'VERIFIED'"
grep VERIFIED *data
