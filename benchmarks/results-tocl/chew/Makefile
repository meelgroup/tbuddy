INTERP=python3
N = 08
TBUDDY = ~/repos/tbuddy
BDIR = $(TBUDDY)/benchmarks
GDIR= $(BDIR)/generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
TBSAT = $(TBUDDY)/tbsat/src/tbsat
CDIR = ~/repos/drat-trim
DRAT = $(CDIR)/drat-trim
LRAT = $(CDIR)/lrat-check
SDIR = ~/repos/kissat/build
KISSAT = $(SDIR)/kissat
KFLAGS = --unsat --no-binary
GRAB = $(TBUDDY)/tools/grab_data.py
LINEARIZE=$(TBUDDY)/tools/linearize.py
RANDOMIZE = $(TBUDDY)/tools/randomizer.py

N = 0010
SEED=123456
VLEVEL=1
RM = rm -f
TLIM=1000

cnf: chew-$(N)-$(SEED).cnf

ldata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).ldata

bdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).bdata

rdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).rdata

kdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).kdata

chew-$(N)-$(SEED).cnf: $(GENERATOR)
	$(GENERATOR) $(N) $(SEED) 
	mv formula.cnf chew-$(N)-$(SEED).cnf

.SUFFIXES: .cnf .ldata .bdata .rdata .kdata 

.cnf.ldata:
	$(TBSAT) -v $(VLEVEL) -t $(TLIM) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.bdata:
	$(TBSAT) -v $(VLEVEL) -b  -t $(TLIM) -i $<  -o $*.lrat | tee $@
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
	$(INTERP) $(GRAB) "Total clauses" *.ldata > chew-tbsat-linear-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.bdata > chew-tbsat-bucket-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.rdata > chew-tbsat-rbucket-clauses.csv
	$(INTERP) $(GRAB) "proof_added" *.kdata > chew-kissat-clauses.csv
	$(INTERP) $(GRAB) "Elapsed" *.ldata > chew-tbsat-linear-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.bdata > chew-tbsat-bucket-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.rdata > chew-tbsat-rbucket-seconds.csv
	$(INTERP) $(GRAB) -1 "process-time" *.kdata > chew-kissat-seconds.csv

	$(INTERP) $(GRAB) "verification time" *.ldata > chew-tbsat-linear-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.bdata > chew-tbsat-bucket-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.rdata > chew-tbsat-rbucket-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.kdata > chew-kissat-check-seconds.csv


clean:
	rm -f chew-*.{cnf,schedule,nq-schedule,order}
	rm -f *.lrat *.drat *~

superclean: clean
	rm -f *.*data




