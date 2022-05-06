INTERP=python3
SIZE = 16
GDIR=../../generators
GENERATOR = $(GDIR)/chess.py
SDIR = ~/repos/kissat/build
SOLVER = $(SDIR)/kissat
CDIR = ~/repos/drat-trim
CHECKER= $(CDIR)/drat-trim
TDIR= ~/repos/tbuddy/tools
GRAB= $(TDIR)/grab_data.py
ROOT=chess-kissat


full: gen run check clear

gen:
	$(INTERP) $(GENERATOR) -n $(SIZE) -r $(ROOT)-$(SIZE)

run:
	$(SOLVER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).drat  | tee $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).drat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).{cnf,drat}

data:
	$(INTERP) $(GRAB) "parsing input formula" *.data > $(ROOT)-input-clauses.csv
	$(INTERP) $(GRAB) 2 "parsing input formula" *.data > $(ROOT)-input-variables.csv
	$(INTERP) $(GRAB) -1 "process-time" *.data > $(ROOT)-sat-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.data > $(ROOT)-check-seconds.csv
	$(INTERP) $(GRAB) "proof_added" *.data > $(ROOT)-proof-clauses.csv

clean:
	rm -f *.{data,drat,order,schedule,cnf,csv}
	rm -f *~





