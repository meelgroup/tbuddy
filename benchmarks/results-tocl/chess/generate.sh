#!/bin/sh

# bucket
make bchess N=008
make bchess N=010
make bchess N=012
make bchess N=014
#make bchess N=016

# linear
make lchess N=008
make lchess N=010
make lchess N=012
make lchess N=014
#make lchess N=016

# noquant
make nqchess N=008
make nqchess N=010
make nqchess N=012
make nqchess N=014
#make nqchess N=016


# kissat
make kchess N=008
make kchess N=010
make kchess N=012
make kchess N=014
#make kchess N=016

# scan
make schess N=008
make schess N=016
make schess N=032
make schess N=064
make schess N=128
make schess N=256
