

install:
	cd buddy/src; make all
	cd bsat/src; make clean; make all
	cd xtree/src; make clean; make all
