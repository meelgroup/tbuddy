INTERP=python3
SIZE = 08
SEED=123456
VLEVEL = 01
VERB=
GDIR=../../generators
GENERATOR = $(GDIR)/urquhart-li-code/generate
PCT=50
SDIR = ../../../src
SOLVER = $(SDIR)/tbsat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/lrat-check
TDIR= ~/repos/tbuddy/tools
PROBER=$(TDIR)/probe.py
WRAP=$(TDIR)/wrap.py
EXTRACT=$(TDIR)/xor_extractor.py
TLIM=10
TLIMM=30
TLIMS=900
TLIML=3600
ROOT=urquhart-gauss

gen:
	$(GENERATOR) -m$(SIZE) -p$(PCT) -f$(ROOT)-$(SIZE).cnf
	$(INTERP) $(EXTRACT) -i $(ROOT)-$(SIZE).cnf -o $(ROOT)-$(SIZE).schedule | tee $(ROOT)-$(SIZE).data

run:
	$(INTERP) $(WRAP) $(TLIM) $(SOLVER) -v $(VLEVEL) -i $(ROOT)-$(SIZE).cnf -s $(ROOT)-$(SIZE).schedule -o $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).lrat

mprobe:
	$(INTERP) $(PROBER) $(VERB) -t $(TLIMM) -m 50 -d 10 -l $(ROOT)-mprobe.data

sprobe:
	$(INTERP) $(PROBER) -t $(TLIMS) -m 50 -d 10 -l $(ROOT)-sprobe.data

lprobe:
	$(INTERP) $(PROBER) -t $(TLIML) -m 50 -d 10 -l $(ROOT)-lprobe.data

clean:
	rm -f $(ROOT)-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f $(ROOT)-*.data




