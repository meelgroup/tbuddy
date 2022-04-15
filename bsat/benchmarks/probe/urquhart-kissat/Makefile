INTERP=python3
SIZE = 08
GDIR=../../generators
GENERATOR = $(GDIR)/urquhart-li-code/generate
PCT=50
SDIR = ~/repos/kissat/build
SOLVER = $(SDIR)/kissat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/drat-trim
TDIR= ~/repos/tbuddy/tools
GRABBER= $(TDIR)/grab_data.py
TPHRASE=process-time
TLIM=10
ROOT=urquhart-kissat

grun: gen run clear

gen:
	$(GENERATOR) -m$(SIZE) -p$(PCT) -f$(ROOT)-$(SIZE).cnf

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




