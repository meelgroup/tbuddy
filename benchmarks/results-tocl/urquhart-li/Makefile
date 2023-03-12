INTERP=python3
M = 03
TBUDDY = ~/repos/tbuddy
BDIR = $(TBUDDY)/benchmarks
GDIR= $(BDIR)/generators
GENERATOR = $(GDIR)/urquhart-li-code/generate
TBSAT = $(TBUDDY)/tbsat/src/tbsat
LRAT = ~/repos/drat-trim/lrat-check
RANDOMIZE = $(TBUDDY)/tools/randomizer.py


PCT=50
VLEVEL=1
RM=rm
TLIM=1000
SEED=1

GRAB = $(INTERP) $(TBUDDY)/tools/grab_data.py

cnf: urquhart-li-$(M).cnf

urquhart-li-$(M).cnf: $(GENERATOR)
	$(GENERATOR) -m$(M) -p$(PCT) -furquhart-li-$(M).cnf

bdata: urquhart-li-$(M).cnf urquhart-li-$(M).bdata

rdata: urquhart-li-$(M).cnf urquhart-li-$(M).rdata

.SUFFIXES: .cnf .bdata .rdata


.cnf.bdata:
	$(TBSAT) -v $(VLEVEL) -t $(TLIM) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.rdata:
	$(RANDOMIZE) $< $(SEED) 
	$(TBSAT) -v $(VLEVEL) -b  -t $(TLIM) -i $< -p $*.order -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

data:
	rm -f *.csv
	$(GRAB) "Elapsed" *.bdata > urquhart-li-tbsat-bucket-seconds.csv
	$(GRAB) "verification time" *.bdata > urquhart-li-tbsat-bucket-check-seconds.csv	
	$(GRAB) "Total clauses" *.bdata > urquhart-li-tbsat-bucket-clauses.csv	
	$(GRAB) "Elapsed" *.rdata > urquhart-li-tbsat-rbucket-seconds.csv
	$(GRAB) "verification time" *.rdata > urquhart-li-tbsat-rbucket-check-seconds.csv	
	$(GRAB) "Total clauses" *.rdata > urquhart-li-tbsat-rbucket-clauses.csv	

clean:
	rm -f urquhart-li-*.cnf
	rm -f *.lrat *~ *.schedule

superclean: clean
	rm -f *data

