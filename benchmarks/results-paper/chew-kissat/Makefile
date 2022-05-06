INTERP=python3
SIZE = 000010
SEED = 123456
VLEVEL=1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ~/repos/kissat/build
SOLVER = $(SDIR)/kissat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/drat-trim
TDIR= ~/repos/tbuddy/tools
GRAB= $(TDIR)/grab_data.py
ROOT=chew-kissat


full: gen run check clear

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
	$(INTERP) $(GRAB) 2 "parsing input formula" *.data > $(ROOT)-input-clauses.csv
	$(INTERP) $(GRAB) 1 "parsing input formula" *.data > $(ROOT)-input-variables.csv
	$(INTERP) $(GRAB) -1 "process-time" *.data > $(ROOT)-sat-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.data > $(ROOT)-check-seconds.csv
	$(INTERP) $(GRAB) "proof_added" *.data > $(ROOT)-proof-clauses.csv




clean:
	rm -f *.{data,drat,order,schedule,cnf,csv}
	rm -f *~




