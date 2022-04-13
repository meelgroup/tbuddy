#!/bin/sh
#pushd chess-pgbdd; make sprobe ; popd
#pushd chess-scan; make sprobe ; popd
#pushd chew-bucket; make sprobe ; popd
#pushd chew-gauss; make sprobe ; popd
#pushd chew-pgbdd; make sprobe ; popd
#pushd chew-pgpbs; make sprobe ; popd
pushd pigeon-pgbdd; make sprobe ; popd
pushd pigeon-scan; make sprobe ; popd
pushd urquhart-bucket; make sprobe ; popd
pushd urquhart-gauss; make sprobe ; popd
pushd urquhart-pgbdd; make sprobe ; popd
pushd urquhart-pgpbs; make sprobe ; popd
