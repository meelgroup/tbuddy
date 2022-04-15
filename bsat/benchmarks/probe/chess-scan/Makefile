INTERP=python3
SIZE = 08
VLEVEL = 01
GDIR=../../generators
GENERATOR = $(GDIR)/chess.py
SDIR = ../../../src
SOLVER = $(SDIR)/tbsat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/lrat-check
TDIR= ~/repos/tbuddy/tools
PROBER=../tprobe.py
TLIM=10
TLIMM=30
TLIMS=900
TLIML=3600
ROOT=chess-scan
MSIZE=50
DSIZE=2

grun: gen run clear

gen:
	$(INTERP) $(GENERATOR) -n $(SIZE) -r $(ROOT)-$(SIZE)

run:
	$(SOLVER) -v $(VLEVEL) -t $(TLIM) -i $(ROOT)-$(SIZE).cnf -s $(ROOT)-$(SIZE).schedule -o $(ROOT)-$(SIZE).lrat | tee $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).{lrat,order,schedule,cnf}

mprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(ROOT) -t $(TLIMM) -m $(MSIZE) -d $(DSIZE) -l $(ROOT)-mprobe.data

sprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(ROOT) -t $(TLIMS) -m $(MSIZE) -d $(DSIZE) -l $(ROOT)-sprobe.data

lprobe:
	$(INTERP) $(PROBER) -v $(VLEVEL) -b $(ROOT) -t $(TLIML) -m $(MSIZE) -d $(DSIZE) -l $(ROOT)-lprobe.data

clean:
	rm -f $(ROOT)-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean




