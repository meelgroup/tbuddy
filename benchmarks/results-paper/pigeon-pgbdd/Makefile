INTERP=python3
SIZE = 015
VLEVEL=1
GDIR=../../generators
GENERATOR=$(GDIR)/pigeon-sinz.py
SDIR = ~/repos/pgbdd/prototype
SOLVER=$(SDIR)/solver.py
CDIR=~/repos/drat-trim
CHECKER=$(CDIR)/lrat-check
TDIR=../../../tools
GRAB=$(TDIR)/grab_data.py
ROOT=pigeon-pgbdd

full: gen run check clear

gen:
	$(INTERP) $(GENERATOR) -n $(SIZE) -r $(ROOT)-$(SIZE)

run:
	$(INTERP) $(SOLVER) -v $(VLEVEL) -i $(ROOT)-$(SIZE).cnf -s $(ROOT)-$(SIZE).schedule -o $(ROOT)-$(SIZE).lrat 2>&1 | tee $(ROOT)-$(SIZE).data

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




