INTERP=python3
SIZE = 08
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
TLIM=10
TLIMM=30
TLIMS=900
TLIML=3600
ROOT=urquhart-bucket

gen:
	$(GENERATOR) -m$(SIZE) -p$(PCT) -f$(ROOT)-$(SIZE).cnf

run:
	$(INTERP) $(WRAP) $(TLIM) $(SOLVER) -v $(VLEVEL) -i $(ROOT)-$(SIZE).cnf -b -o $(ROOT)-$(SIZE).lrat | tee $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).lrat

mprobe:
	$(INTERP) $(PROBER) $(VERB) -t $(TLIMM) -m 10 -d 1 -l $(ROOT)-mprobe.data

sprobe:
	$(INTERP) $(PROBER) -t $(TLIMS) -m 10 -d 1 -l $(ROOT)-sprobe.data

lprobe:
	$(INTERP) $(PROBER) -t $(TLIML) -m 10 -d 1 -l $(ROOT)-lprobe.data

clean:
	rm -f $(ROOT)-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f $(ROOT)-*.data




