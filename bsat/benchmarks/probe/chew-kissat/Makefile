INTERP=python3
SIZE = 08
SEED=123456
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ~/repos/kissat/build
SOLVER = $(SDIR)/kissat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/drat-trim
TDIR= ~/repos/tbuddy/tools
GRABBER= $(TDIR)/grab_data.py
TPHRASE=process-time
TLIM=10
BENCH=chew-kissat
ROOT=$(BENCH)-$(SEED)

grun: gen run clear

gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(ROOT)-$(SIZE).cnf

run:
	$(SOLVER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).drat  | tee $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).drat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).{drat,cnf,order,schedule}

data:
	$(INTERP) $(GRABBER) 2 $(TPHRASE) *.data > $(ROOT).csv

clean:
	rm -f $(ROOT)-*.{cnf,order,schedule}
	rm -f *.drat *~

superclean: clean




