INTERP=python3
SIZE = 08
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

grun: gen run clear

gen:
	$(GENERATOR) $(SIZE) 1
	mv formula.cnf $(BENCH)-1-$(SIZE).cnf
	$(GENERATOR) $(SIZE) 2
	mv formula.cnf $(BENCH)-2-$(SIZE).cnf
	$(GENERATOR) $(SIZE) 3
	mv formula.cnf $(BENCH)-3-$(SIZE).cnf
	$(GENERATOR) $(SIZE) 4
	mv formula.cnf $(BENCH)-4-$(SIZE).cnf
	$(GENERATOR) $(SIZE) 5
	mv formula.cnf $(BENCH)-5-$(SIZE).cnf

run:
	$(SOLVER) $(BENCH)-1-$(SIZE).cnf $(BENCH)-1-$(SIZE).drat  | tee $(BENCH)-1-$(SIZE).data
	$(SOLVER) $(BENCH)-2-$(SIZE).cnf $(BENCH)-2-$(SIZE).drat  | tee $(BENCH)-2-$(SIZE).data
	$(SOLVER) $(BENCH)-3-$(SIZE).cnf $(BENCH)-3-$(SIZE).drat  | tee $(BENCH)-3-$(SIZE).data
	$(SOLVER) $(BENCH)-4-$(SIZE).cnf $(BENCH)-4-$(SIZE).drat  | tee $(BENCH)-4-$(SIZE).data
	$(SOLVER) $(BENCH)-5-$(SIZE).cnf $(BENCH)-5-$(SIZE).drat  | tee $(BENCH)-5-$(SIZE).data

check:
	$(CHECKER) $(BENCH)-1-$(SIZE).cnf $(BENCH)-1-$(SIZE).drat | tee -a $(BENCH)-1-$(SIZE).data
	$(CHECKER) $(BENCH)-2-$(SIZE).cnf $(BENCH)-2-$(SIZE).drat | tee -a $(BENCH)-2-$(SIZE).data
	$(CHECKER) $(BENCH)-3-$(SIZE).cnf $(BENCH)-3-$(SIZE).drat | tee -a $(BENCH)-3-$(SIZE).data
	$(CHECKER) $(BENCH)-4-$(SIZE).cnf $(BENCH)-4-$(SIZE).drat | tee -a $(BENCH)-4-$(SIZE).data
	$(CHECKER) $(BENCH)-5-$(SIZE).cnf $(BENCH)-5-$(SIZE).drat | tee -a $(BENCH)-5-$(SIZE).data

clear:
	rm -f $(BENCH)-*-$(SIZE).{drat,cnf,order,schedule}

data:
	$(INTERP) $(GRABBER) 2 $(TPHRASE) *.data > $(BENCH).csv

clean:
	rm -f $(BENCH)-*.{cnf,order,schedule}
	rm -f *.drat *~

superclean: clean




