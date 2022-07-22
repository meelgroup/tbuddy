#!/bin/sh

pushd chess-scan-noproof ;  make full SIZE=018 ; popd
pushd chess-scan-noproof ;  make full SIZE=150 ; popd
pushd chess-scan-noproof ;  make full SIZE=368 ; popd

pushd pigeon-scan-noproof ;  make full SIZE=014 ; popd
pushd pigeon-scan-noproof ;  make full SIZE=150 ; popd
pushd pigeon-scan-noproof ;  make full SIZE=254 ; popd

#pushd chew-bucket-noproof ;  make full SIZE=00044 ; popd
#pushd chew-bucket-noproof ;  make full SIZE=05000 ; popd
#pushd chew-bucket-noproof ;  make full SIZE=08666 ; popd

#pushd chew-gauss-noproof ;  make full SIZE=008666 ; popd
#pushd chew-gauss-noproof ;  make full SIZE=010000 ; popd
#pushd chew-gauss-noproof ;  make full SIZE=699051 ; popd

pushd urquhart-bucket-noproof ;  make full SIZE=003 ; popd
pushd urquhart-bucket-noproof ;  make full SIZE=026 ; popd
pushd urquhart-bucket-noproof ;  make full SIZE=035 ; popd

pushd urquhart-gauss-noproof ;  make full SIZE=035 ; popd
pushd urquhart-gauss-noproof ;  make full SIZE=048 ; popd
pushd urquhart-gauss-noproof ;  make full SIZE=316 ; popd


