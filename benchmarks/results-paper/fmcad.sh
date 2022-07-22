#!/bin/sh

pushd chew-pgbdd ; make full SIZE=02000 ; popd
pushd chew-bucket ; make full SIZE=02000 ; popd
pushd chew-pgpbs ; make full SIZE=05000 ; popd
pushd chew-gauss ; make full SIZE=05000 ; popd
pushd urquhart-pgbdd ; make full SIZE=018 ; popd
pushd urquhart-bucket ; make full SIZE=018 ; popd
pushd urquhart-pgpbs ; make full SIZE=028 ; popd
pushd urquhart-gauss ; make full SIZE=028 ; popd
pushd chess-pgbdd ; make full SIZE=80 ; popd
pushd chess-scan ; make full SIZE=80 ; popd
pushd pigeon-pgbdd ; make full SIZE=90 ; popd
pushd pigeon-scan ; make full SIZE=90 ; popd
