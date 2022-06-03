INTERP=python3
SIZE = 010
VLEVEL=1
FDIR = ../../files/gocht-tseitin
SDIR = ../../../tbsat/src
SOLVER=$(SDIR)/tbsat
CDIR=~/repos/drat-trim
CHECKER=$(CDIR)/lrat-check
TDIR=../../../tools
GRAB=$(TDIR)/grab_data.py
ROOT=gocht-tseitin-bucket

full: gen run check clear

gen:
	cp $(FDIR)/tseitin_n$(SIZE)_d5.cnf $(ROOT)-$(SIZE).cnf
	ls -l $(ROOT)-$(SIZE).cnf | tee -a $(ROOT)-$(SIZE).data

run:
	$(SOLVER) -b -v $(VLEVEL) -i $(ROOT)-$(SIZE).cnf -o $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data
	ls -l $(ROOT)-$(SIZE).lrat | tee -a $(ROOT)-$(SIZE).data

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





