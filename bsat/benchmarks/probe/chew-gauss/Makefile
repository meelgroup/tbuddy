INTERP=python3
SIZE = 08
SEED=123456
VLEVEL = 1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ../../../src
SOLVER = $(SDIR)/tbsat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/lrat-check
TDIR= ~/repos/tbuddy/tools
PROBER=../tprobe.py
EXTRACT=$(TDIR)/xor_extractor.py
TLIM=10
TLIMM=30
TLIMS=900
TLIML=3600
BENCH=chew-gauss
ROOT=$(BENCH)-$(SEED)
MSIZE=5000
LSIZE=699051
HSIZE=699052
DSIZE=1

grun: gen run clear


gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(ROOT)-$(SIZE).cnf
	$(INTERP) $(EXTRACT) -i $(ROOT)-$(SIZE).cnf -o $(ROOT)-$(SIZE).schedule | tee $(ROOT)-$(SIZE).data

run:
	$(SOLVER) -v $(VLEVEL) -t $(TLIM) -i $(ROOT)-$(SIZE).cnf -s $(ROOT)-$(SIZE).schedule -o $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).{lrat,order,schedule,cnf}

mprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(BENCH) -t $(TLIMM) -m $(MSIZE) -d $(DSIZE) -s $(SEED) -l $(ROOT)-mprobe.data

sprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(BENCH) -t $(TLIMS) -m $(LSIZE) -u $(HSIZE) -d $(DSIZE) -s $(SEED) -l $(ROOT)-sprobe.data

lprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(BENCH) -t $(TLIML) -m $(LSIZE) -u $(HSIZE) -d $(DSIZE) -s $(SEED) -l $(ROOT)-lprobe.data

clean:
	rm -f $(ROOT)-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f $(ROOT)-*.data




