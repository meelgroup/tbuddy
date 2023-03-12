INTERP=python3
TBUDDY = ~/repos/tbuddy
BDIR = $(TBUDDY)/benchmarks
GDIR= $(BDIR)/generators
GENERATOR = $(GDIR)/urquhart-simon-code/genurq
TBSAT = $(TBUDDY)/tbsat/src/tbsat
RANDOMIZE = $(TBUDDY)/tools/randomizer.py
LRAT = ~/repos/drat-trim/lrat-check
DRAT = ~/repos/drat-trim/drat-trim
SDIR = ~/repos/kissat/build
KISSAT = $(SDIR)/kissat
KFLAGS = --unsat --no-binary

SEED=123456
M = 03
VLEVEL=1
RM=rm -f
TLIM=1000

GRAB = $(INTERP) $(TBUDDY)/tools/grab_data.py

cnf: urquhart-simon-$(M)-$(SEED).cnf

urquhart-simon-$(M)-$(SEED).cnf: $(GENERATOR)
	$(GENERATOR) $(M) $(SEED) > urquhart-simon-$(M)-$(SEED).cnf

bdata: urquhart-simon-$(M)-$(SEED).cnf urquhart-simon-$(M)-$(SEED).bdata

rdata: urquhart-simon-$(M)-$(SEED).cnf urquhart-simon-$(M)-$(SEED).rdata

kdata: urquhart-simon-$(M)-$(SEED).cnf urquhart-simon-$(M)-$(SEED).kdata

.SUFFIXES: .cnf .bdata .rdata .kdata

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

.cnf.kdata:
	$(KISSAT) $(KFLAGS) --time=$(TLIM) $< $*.drat | tee $@
	ls -l $*.drat | tee -a $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat

data:
	rm -f *.csv
	$(GRAB) "Elapsed" *.bdata > urquhart-simon-tbsat-bucket-seconds.csv
	$(GRAB) "verification time" *.bdata > urquhart-simon-tbsat-bucket-check-seconds.csv
	$(GRAB) "Total clauses" *.bdata > urquhart-simon-tbsat-bucket-clauses.csv	
	$(GRAB) "Elapsed" *.rdata > urquhart-simon-tbsat-rbucket-seconds.csv
	$(GRAB) "verification time" *.rdata > urquhart-simon-rtbsat-bucket-check-seconds.csv
	$(GRAB) "Total clauses" *.rdata > urquhart-simon-tbsat-rbucket-clauses.csv	
	$(INTERP) $(GRAB) "proof_added" *.kdata > urquhart-li-kissat-clauses.csv
	$(INTERP) $(GRAB) -1 "process-time" *.kdata > urquhart-li-kissat-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.kdata > urquhart-li-kissat-check-seconds.csv


clean:
	rm -f urquhart-simon-*.cnf
	rm -f *.lrat *~ *.schedule *.order

superclean: clean
	rm -f *data

