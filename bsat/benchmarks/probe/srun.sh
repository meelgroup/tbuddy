#!/bin/sh
pushd chess-pgbdd; make grun SIZE=80  TLIM=5000 ; popd
pushd chess-scan; make grun SIZE=320  TLIM=5000 ; popd

pushd pigeon-pgbdd; make grun SIZE=80  TLIM=5000 ; popd
pushd pigeon-scan; make grun SIZE=192  TLIM=5000 ; popd

pushd chew-pgbdd; make grun SIZE=2800  TLIM=5000 ; popd
pushd chew-bucket; make grun SIZE=10000  TLIM=5000 ; popd
pushd chew-pgpbs; make grun SIZE=5000  TLIM=5000 ; popd
pushd chew-gauss; make grun SIZE=699051  TLIM=5000 ; popd

pushd urquhart-pgbdd; make grun SIZE=16  TLIM=5000 ; popd
pushd urquhart-bucket; make grun SIZE=30  TLIM=5000 ; popd
pushd urquhart-pgpbs; make grun SIZE=28  TLIM=5000 ; popd
pushd urquhart-gauss; make grun SIZE=316  TLIM=5000 ; popd


