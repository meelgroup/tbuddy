INTERP=python3
SIZE = 08
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ~/repos/kissat/build
SOLVER = $(SDIR)/kissat
SFLAGS = --unsat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/drat-trim
TDIR= ~/repos/tbuddy/tools
GRABBER= $(TDIR)/grab_data.py
TPHRASE=process-time
TLIM=4500
BENCH=chew-kissat

grun: gen run clear

final: gen run check clear

gen:
	$(GENERATOR) $(SIZE) 1
	mv formula.cnf $(BENCH)-$(SIZE)-1.cnf
	$(GENERATOR) $(SIZE) 2
	mv formula.cnf $(BENCH)-$(SIZE)-2.cnf
	$(GENERATOR) $(SIZE) 3
	mv formula.cnf $(BENCH)-$(SIZE)-3.cnf
	$(GENERATOR) $(SIZE) 4
	mv formula.cnf $(BENCH)-$(SIZE)-4.cnf
	$(GENERATOR) $(SIZE) 5
	mv formula.cnf $(BENCH)-$(SIZE)-5.cnf

run:
	$(SOLVER) $(SFLAGS) $(BENCH)-$(SIZE)-1.cnf $(BENCH)-$(SIZE)-1.drat --time=$(TLIM)  | tee $(BENCH)-$(SIZE)-1.data
	$(SOLVER) $(SFLAGS) $(BENCH)-$(SIZE)-2.cnf $(BENCH)-$(SIZE)-2.drat --time=$(TLIM)  | tee $(BENCH)-$(SIZE)-2.data
	$(SOLVER) $(SFLAGS) $(BENCH)-$(SIZE)-3.cnf $(BENCH)-$(SIZE)-3.drat --time=$(TLIM)  | tee $(BENCH)-$(SIZE)-3.data
	$(SOLVER) $(SFLAGS) $(BENCH)-$(SIZE)-4.cnf $(BENCH)-$(SIZE)-4.drat --time=$(TLIM)  | tee $(BENCH)-$(SIZE)-4.data
	$(SOLVER) $(SFLAGS) $(BENCH)-$(SIZE)-5.cnf $(BENCH)-$(SIZE)-5.drat --time=$(TLIM)  | tee $(BENCH)-$(SIZE)-5.data

check:
	$(CHECKER) $(BENCH)-$(SIZE)-1.cnf $(BENCH)-$(SIZE)-1.drat | tee -a $(BENCH)-$(SIZE)-1.data
	$(CHECKER) $(BENCH)-$(SIZE)-2.cnf $(BENCH)-$(SIZE)-2.drat | tee -a $(BENCH)-$(SIZE)-2.data
	$(CHECKER) $(BENCH)-$(SIZE)-3.cnf $(BENCH)-$(SIZE)-3.drat | tee -a $(BENCH)-$(SIZE)-3.data
	$(CHECKER) $(BENCH)-$(SIZE)-4.cnf $(BENCH)-$(SIZE)-4.drat | tee -a $(BENCH)-$(SIZE)-4.data
	$(CHECKER) $(BENCH)-$(SIZE)-5.cnf $(BENCH)-$(SIZE)-5.drat | tee -a $(BENCH)-$(SIZE)-5.data

clear:
	rm -f $(BENCH)-$(SIZE)-*.{drat,cnf,order,schedule}

data:
	$(INTERP) $(GRABBER) 2 $(TPHRASE) *.data > $(BENCH).csv

clean:
	rm -f $(BENCH)-*.{cnf,order,schedule}
	rm -f *.drat *~

superclean: clean




