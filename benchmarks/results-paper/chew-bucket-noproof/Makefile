INTERP=python3
SIZE = 000050
SEED = 123456
VLEVEL=1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ../../../tbsat/src
SOLVER=$(SDIR)/tbsat
TDIR=../../../tools
GRAB=$(TDIR)/grab_data.py
ROOT=chew-bucket-noproof

full: gen run clear

gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(ROOT)-$(SIZE).cnf

run:
	$(SOLVER) -v $(VLEVEL) -b -i $(ROOT)-$(SIZE).cnf | tee $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).{cnf,lrat,order,schedule}

data:
	$(INTERP) $(GRAB) "Read" *.data > $(ROOT)-input-clauses.csv
	$(INTERP) $(GRAB) 2 "Read" *.data > $(ROOT)-input-variables.csv
	$(INTERP) $(GRAB) "Elapsed" *.data > $(ROOT)-sat-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.data > $(ROOT)-check-seconds.csv
	$(INTERP) $(GRAB) "Total clauses" *.data > $(ROOT)-proof-clauses.csv


clean:
	rm -f *.{data,lrat,order,schedule,cnf,csv}
	rm -f *~





