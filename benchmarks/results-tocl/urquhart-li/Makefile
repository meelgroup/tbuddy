INTERP=python3
M = 03
TBUDDY = ~/repos/tbuddy
BDIR = $(TBUDDY)/benchmarks
GDIR= $(BDIR)/generators
GENERATOR = $(GDIR)/urquhart-li-code/generate
TBSAT = ../../../tbsat/src/tbsat
TBSAT = $(TBUDDY)/tbsat/src/tbsat
LRAT = ~/repos/drat-trim/lrat-check

PCT=50
VLEVEL=1
RM=rm
TLIM=1000

GRAB = $(INTERP) $(TOOLDIR)/grab_data.py

cnf: urquhart-li-$(M).cnf

urquhart-li-$(M).cnf: $(GENERATOR)
	$(GENERATOR) -m$(M) -p$(PCT) -furquhart-li-$(M).cnf

bdata: urquhart-li-$(M).bdata

.SUFFIXES: .cnf .bdata


.cnf.bdata:
	$(TBSAT) -v $(VLEVEL) -t $(TLIM) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

data:
	rm -f *.csv
	$(GRAB) "Elapsed" *.bdata > urquhart-li-tbsat-bucket-seconds.csv
	$(GRAB) "verification time" *.bdata > urquhart-li-tbsat-bucket-check-seconds.csv	
	$(GRAB) "Total clauses" *.bdata > urquhart-li-tbsat-bucket-clauses.csv	

clean:
	rm -f urquhart-li-*.cnf
	rm -f *.lrat *~ *.schedule

superclean: clean
	rm -f *data

