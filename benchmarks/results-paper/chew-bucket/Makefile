INTERP=python3
SIZE = 000050
SEED = 123456
VLEVEL=1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ../../../tbsat/src
SOLVER=$(SDIR)/tbsat
CDIR=~/repos/drat-trim
CHECKER=$(CDIR)/lrat-check
TDIR=../../../tools
GRAB=$(TDIR)/grab_data.py
ROOT=chew-bucket

full: gen run check clear

gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(ROOT)-$(SIZE).cnf

run:
	$(SOLVER) -v $(VLEVEL) -b -i $(ROOT)-$(SIZE).cnf -o $(ROOT)-$(SIZE).lrat | tee $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).{lrat,order,schedule}

data:
	$(INTERP) $(GRAB) "Read" *.data > $(ROOT)-input-clauses.csv
	$(INTERP) $(GRAB) 2 "Read" *.data > $(ROOT)-input-variables.csv
	$(INTERP) $(GRAB) "Elapsed" *.data > $(ROOT)-sat-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.data > $(ROOT)-check-seconds.csv
	$(INTERP) $(GRAB) "Total clauses" *.data > $(ROOT)-proof-clauses.csv


clean:
	rm -f *.{data,lrat,order,schedule,cnf,csv}
	rm -f *~





