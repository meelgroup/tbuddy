INTERP=python3
SIZE = 000050
SEED = 123456
VLEVEL=1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
SDIR = ../../../tbsat/src
SOLVER=$(SDIR)/tbsat
TDIR=../../../tools
EXTRACT=$(TDIR)/xor_extractor.py
GRAB=$(TDIR)/grab_data.py
ROOT=chew-gauss-noproof

full: gen run clear

gen:
	$(GENERATOR) $(SIZE) $(SEED) 
	mv formula.cnf $(ROOT)-$(SIZE).cnf
	$(INTERP) $(EXTRACT) -i $(ROOT)-$(SIZE).cnf -o $(ROOT)-$(SIZE).schedule | tee $(ROOT)-$(SIZE).data

run:
	$(SOLVER) -v $(VLEVEL) -i $(ROOT)-$(SIZE).cnf -s $(ROOT)-$(SIZE).schedule | tee -a $(ROOT)-$(SIZE).data

check:
	$(CHECKER) $(ROOT)-$(SIZE).cnf $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

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





