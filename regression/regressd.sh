#!/bin/sh

make 
rm -f *.lddata *.sddata *.bddata

make 01-unsat2.lddata
make 01-unsat2.bddata
make 02-xorImpliesOr3.lddata
make 02-xorImpliesOr3.bddata
make 03-chew-10.bddata
make 04-chess-32.sddata
make 05-pigeon-sinz-32.sddata
make 06-urquhart-li-09.bddata

echo "\nBelow, should get 8 lines of 'VERIFIED'"
grep VERIFIED *ddata
