INTERP=python3
SIZE = 10
WIDTH=5
SEED=1
RSEED=1
VLEVEL=1
FDIR = ../../files/cnfgen-tseitin
SDIR = ../../../tbsat/src
SOLVER=$(SDIR)/tbsat
CDIR=~/repos/drat-trim
CHECKER=$(CDIR)/lrat-check
TDIR=../../../tools
EXTRACT=$(TDIR)/xor_extractor.py
GRAB=$(TDIR)/grab_data.py
ROOT=cnfgen-tseitin-gauss
TLIM=600

quick: gen run clear

full: gen run check clear

gen:
	cp $(FDIR)/tseitin-$(SIZE)-w$(WIDTH)-s$(SEED).cnf $(ROOT)-$(SIZE)-r$(RSEED).cnf
	$(INTERP) $(EXTRACT) -i $(ROOT)-$(SIZE)-r$(RSEED).cnf -o $(ROOT)-$(SIZE)-r$(RSEED).schedule | tee $(ROOT)-$(SIZE)-r$(RSEED).data
	ls -l $(ROOT)-$(SIZE)-r$(RSEED).cnf | tee -a $(ROOT)-$(SIZE)-r$(RSEED).data

run:
	$(SOLVER) -v $(VLEVEL) -i $(ROOT)-$(SIZE)-r$(RSEED).cnf -r $(RSEED) -t $(TLIM) -s $(ROOT)-$(SIZE)-r$(RSEED).schedule -o $(ROOT)-$(SIZE)-r$(RSEED).lrat | tee -a $(ROOT)-$(SIZE)-r$(RSEED).data
	ls -l $(ROOT)-$(SIZE)-r$(RSEED).lrat | tee -a $(ROOT)-$(SIZE)-r$(RSEED).data

check:
	$(CHECKER) $(ROOT)-$(SIZE)-r$(RSEED).cnf $(ROOT)-$(SIZE)-r$(RSEED).lrat | tee -a $(ROOT)-$(SIZE)-r$(RSEED).data

clear:
	rm -f $(ROOT)-$(SIZE)-r$(RSEED).{cnf,lrat,order,schedule}

data:
#	$(INTERP) $(GRAB) "Read" $(ROOT)-*.data > $(ROOT)-input-clauses.csv
#	$(INTERP) $(GRAB) 2 "Read" $(ROOT)-*.data > $(ROOT)-input-variables.csv
	$(INTERP) $(GRAB) "Elapsed" $(ROOT)-*.data > $(ROOT)-sat-seconds.csv
#	$(INTERP) $(GRAB) "verification time" $(ROOT)-*.data > $(ROOT)-check-seconds.csv
#	$(INTERP) $(GRAB) "Total clauses" $(ROOT)-*.data > $(ROOT)-proof-clauses.csv
	$(INTERP) $(GRAB) "Sum of constraint sizes" $(ROOT)-*.data > $(ROOT)-constraints.csv
	$(INTERP) $(GRAB) "Operator Miss" $(ROOT)-*.data > $(ROOT)-bdd-ops.csv


clean:
	rm -f *.{data,lrat,order,schedule,cnf,csv}
	rm -f *~





