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
TLIM=10
TLIMM=30
TLIMS=900
TLIML=3600
ROOT=chew-pgbdd-$(SEED)

gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(ROOT)-$(SIZE).cnf

run:
	$(INTERP) $(WRAP) $(TLIM) $(SOLVER) -v $(VLEVEL) -i $(ROOT)-$(SIZE).cnf -b -o $(ROOT)-$(SIZE).lrat | tee $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).lrat

mprobe:
	$(INTERP) $(PROBER) $(VERB) -t $(TLIMM) -m 500 -d 100 -l $(ROOT)-mprobe.data

sprobe:
	$(INTERP) $(PROBER) -t $(TLIMS) -m 500 -d 100 -l $(ROOT)-sprobe.data

lprobe:
	$(INTERP) $(PROBER) -t $(TLIML) -m 500 -d 100 -l $(ROOT)-lprobe.data

clean:
	rm -f $(ROOT)-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean





