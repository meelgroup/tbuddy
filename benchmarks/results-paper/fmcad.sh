#!/bin/sh

pushd chew-pgbdd ; make full SIZE=05000 ; popd
pushd chew-bucket ; make full SIZE=05000 ; popd
pushd chew-pgpbs ; make full SIZE=10000 ; popd
pushd chew-gauss ; make full SIZE=10000 ; popd
pushd urquhart-pgbdd ; make full SIZE=026 ; popd
pushd urquhart-bucket ; make full SIZE=026 ; popd
pushd urquhart-pgpbs ; make full SIZE=048 ; popd
pushd urquhart-gauss ; make full SIZE=048 ; popd
pushd chess-pgbdd ; make full SIZE=150 ; popd
pushd chess-scan ; make full SIZE=150 ; popd
pushd pigeon-pgbdd ; make full SIZE=150 ; popd
pushd pigeon-scan ; make full SIZE=150 ; popd
