INTERP=python3
SIZE = 08
SEED=123456
VLEVEL = 01
VERB=
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ~/repos/pgbdd/prototype
SOLVER = $(SDIR)/solver.py
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/lrat-check
TDIR= ~/repos/tbuddy/tools
PROBER=$(TDIR)/probe.py
WRAP=$(TDIR)/wrap.py
EDIR= ~/repos/pgbdd/extractor
EXTRACT=$(EDIR)/xor_extractor.py
TLIM=10
TLIMM=30
TLIMS=900
TLIML=3600
ROOT=chew-pgpbs-$(SEED)
MSIZE=1000
DSIZE=1

gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(ROOT)-$(SIZE).cnf
	$(INTERP) $(EXTRACT) -i $(ROOT)-$(SIZE).cnf -o $(ROOT)-$(SIZE).schedule | tee $(ROOT)-$(SIZE).data

run:
	$(INTERP) $(WRAP) $(TLIM) $(SOLVER) -v $(VLEVEL) -i $(ROOT)-$(SIZE).cnf -s $(ROOT)-$(SIZE).schedule -o $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).lrat

mprobe:
	$(INTERP) $(PROBER) $(VERB) -t $(TLIMM) -m $(MSIZE) -d $(DSIZE) -l $(ROOT)-mprobe.data

sprobe:
	$(INTERP) $(PROBER) -t $(TLIMS) -m $(MSIZE) -d $(DSIZE) -l $(ROOT)-sprobe.data

lprobe:
	$(INTERP) $(PROBER) -t $(TLIML) -m $(MSIZE) -d $(DSIZE) -l $(ROOT)-lprobe.data

clean:
	rm -f $(ROOT)-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f $(ROOT)-*.data




