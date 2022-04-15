#!/bin/sh
pushd chess-pgbdd; make grun SIZE=100  TLIM=5000 ; popd
pushd chess-scan; make grun SIZE=400  TLIM=5000 ; popd

pushd chew-bucket; make grun SIZE=7000  TLIM=5000 ; popd
#pushd chew-gauss; make grun SIZE=699051  TLIM=5000 ; popd
pushd chew-pgbdd; make grun SIZE=2000  TLIM=5000 ; popd
pushd chew-pgpbs; make grun SIZE=3000  TLIM=5000 ; popd

pushd pigeon-pgbdd; make grun SIZE=100  TLIM=5000 ; popd
pushd pigeon-scan; make grun SIZE=260  TLIM=5000 ; popd

pushd urquhart-bucket; make grun SIZE=40  TLIM=5000 ; popd
pushd urquhart-gauss; make grun SIZE=316  TLIM=5000 ; popd
pushd urquhart-pgbdd; make grun SIZE=18  TLIM=5000 ; popd
pushd urquhart-pgpbs; make grun SIZE=20  TLIM=5000 ; popd
