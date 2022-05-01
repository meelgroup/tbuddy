INTERP=python3
SIZE = 08
SEED=123456
VLEVEL = 1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ~/repos/pgbdd/prototype
SOLVER = $(SDIR)/solver.py
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/lrat-check
TDIR= ~/repos/tbuddy/tools
PROBER=../tprobe.py
EDIR= ~/repos/pgbdd/extractor
EXTRACT=$(EDIR)/xor_extractor.py
TLIM=10
TLIMM=30
TLIMS=900
TLIML=3600
BENCH=chew-pgpbs-$(INTERP)
ROOT=$(BENCH)-$(SEED)
MSIZE=1000
LSIZE=9000
HSIZE=12000
DSIZE=1
NZLIM=800000

grun: gen run clear

final: gen run check clear

grun5: gen5 run5 clear5

gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(BENCH)-$(SIZE)-$(SEED).cnf
	$(INTERP) $(EXTRACT) -r $(SEED) -i $(BENCH)-$(SIZE)-$(SEED).cnf -o $(BENCH)-$(SIZE)-$(SEED).schedule | tee $(BENCH)-$(SIZE)-$(SEED).data

run:
	$(INTERP) $(SOLVER) -v $(VLEVEL) -r $(SEED) -t $(TLIM) -Z $(NZLIM) -i $(BENCH)-$(SIZE)-$(SEED).cnf -s $(BENCH)-$(SIZE)-$(SEED).schedule -o $(BENCH)-$(SIZE)-$(SEED).lrat 2>&1 | tee -a $(BENCH)-$(SIZE)-$(SEED).data

gen5:
	$(GENERATOR) $(SIZE) 1 
	mv formula.cnf $(BENCH)-$(SIZE)-1.cnf
	$(INTERP) $(EXTRACT) -r $(SEED) -i $(BENCH)-$(SIZE)-1.cnf -o $(BENCH)-$(SIZE)-1.schedule | tee $(BENCH)-$(SIZE)-1.data
	$(GENERATOR) $(SIZE) 2 
	mv formula.cnf $(BENCH)-$(SIZE)-2.cnf
	$(INTERP) $(EXTRACT) -r $(SEED) -i $(BENCH)-$(SIZE)-2.cnf -o $(BENCH)-$(SIZE)-2.schedule | tee $(BENCH)-$(SIZE)-2.data
	$(GENERATOR) $(SIZE) 3 
	mv formula.cnf $(BENCH)-$(SIZE)-3.cnf
	$(INTERP) $(EXTRACT) -r $(SEED) -i $(BENCH)-$(SIZE)-3.cnf -o $(BENCH)-$(SIZE)-3.schedule | tee $(BENCH)-$(SIZE)-3.data
	$(GENERATOR) $(SIZE) 4 
	mv formula.cnf $(BENCH)-$(SIZE)-4.cnf
	$(INTERP) $(EXTRACT) -r $(SEED) -i $(BENCH)-$(SIZE)-4.cnf -o $(BENCH)-$(SIZE)-4.schedule | tee $(BENCH)-$(SIZE)-4.data
	$(GENERATOR) $(SIZE) 5 
	mv formula.cnf $(BENCH)-$(SIZE)-5.cnf
	$(INTERP) $(EXTRACT) -r $(SEED) -i $(BENCH)-$(SIZE)-5.cnf -o $(BENCH)-$(SIZE)-5.schedule | tee $(BENCH)-$(SIZE)-5.data


run5:
	$(SOLVER) -v $(VLEVEL) -r 1 -t $(TLIM) -Z $(NZLIM) -i $(BENCH)-$(SIZE)-1.cnf -s $(BENCH)-$(SIZE)-1.schedule -o $(BENCH)-$(SIZE)-1.lrat 2>&1 | tee -a $(BENCH)-$(SIZE)-1.data
	$(SOLVER) -v $(VLEVEL) -r 2 -t $(TLIM) -Z $(NZLIM) -i $(BENCH)-$(SIZE)-2.cnf -s $(BENCH)-$(SIZE)-2.schedule -o $(BENCH)-$(SIZE)-2.lrat 2>&1 | tee -a $(BENCH)-$(SIZE)-2.data
	$(SOLVER) -v $(VLEVEL) -r 3 -t $(TLIM) -Z $(NZLIM) -i $(BENCH)-$(SIZE)-3.cnf -s $(BENCH)-$(SIZE)-3.schedule -o $(BENCH)-$(SIZE)-3.lrat 2>&1 | tee -a $(BENCH)-$(SIZE)-3.data
	$(SOLVER) -v $(VLEVEL) -r 4 -t $(TLIM) -Z $(NZLIM) -i $(BENCH)-$(SIZE)-4.cnf -s $(BENCH)-$(SIZE)-4.schedule -o $(BENCH)-$(SIZE)-4.lrat 2>&1 | tee -a $(BENCH)-$(SIZE)-4.data
	$(SOLVER) -v $(VLEVEL) -r 5 -t $(TLIM) -Z $(NZLIM) -i $(BENCH)-$(SIZE)-5.cnf -s $(BENCH)-$(SIZE)-5.schedule -o $(BENCH)-$(SIZE)-5.lrat 2>&1 | tee -a $(BENCH)-$(SIZE)-5.data

clear5:
	rm -f $(BENCH)-$(SIZE)-*.{lrat}


check:
	$(CHECKER) $(BENCH)-$(SIZE)-$(SEED).cnf $(BENCH)-$(SIZE)-$(SEED).lrat | tee -a $(BENCH)-$(SIZE)-$(SEED).data

clear:
	rm -f $(BENCH)-$(SIZE)-*.{lrat}

mprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(BENCH) -t $(TLIMM) -m $(MSIZE) -d $(DSIZE) -s $(SEED) -l $(BENCH)-mprobe.data

sprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(BENCH) -t $(TLIMS) -m $(LSIZE) -u $(HSIZE) -d $(DSIZE) -s $(SEED) -l $(BENCH)-sprobe.data

lprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(BENCH) -t $(TLIML) -m $(LSIZE) -u $(HSIZE) -d $(DSIZE) -s $(SEED) -l $(BENCH)-lprobe.data

clean:
	rm -f *.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f $(BENCH)-*.data




