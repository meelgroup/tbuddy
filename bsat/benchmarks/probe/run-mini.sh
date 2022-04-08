#!/bin/sh
pushd chess-pgbdd; make mprobe ; popd
pushd chess-scan; make mprobe ; popd
pushd chew-bucket; make mprobe ; popd
pushd chew-gauss; make mprobe ; popd
pushd chew-pgbdd; make mprobe ; popd
pushd chew-pgpbs; make mprobe ; popd
pushd pigeon-pgbdd; make mprobe ; popd
pushd pigeon-scan; make mprobe ; popd
pushd run-short.sh; make mprobe ; popd
pushd urquhart-bucket; make mprobe ; popd
pushd urquhart-gauss; make mprobe ; popd
pushd urquhart-pgbdd; make mprobe ; popd
pushd urquhart-pgpbs; make mprobe ; popd
