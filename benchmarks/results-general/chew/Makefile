INTERP=python3
N = 10
SEED=1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
KDIR=~/repos/kissat/build
KISSAT=$(KDIR)/kissat
KFLAGS = --unsat --no-binary
TBSAT = ../../../tbsat/src/tbsat
TOOLDIR = ../../../tools
EXTRACT = $(TOOLDIR)/xor_extractor.py
GRAB = $(TOOLDIR)/grab_data.py
RANDOMIZE = $(TOOLDIR)/randomizer.py
DDIR=~/repos/drat-trim
DRAT= $(DDIR)/drat-trim
LRAT = $(DDIR)/lrat-check
VLEVEL=1
RM=rm

cnf: chew-$(N)-$(SEED).cnf

kdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).kdata

ldata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).ldata

bdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).bdata

gdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).gdata


chew-$(N)-$(SEED).cnf: $(GENERATOR)
	$(GENERATOR) $(N) $(SEED) 
	mv formula.cnf chew-$(N)-$(SEED).cnf

.SUFFIXES: .cnf .kdata .ldata .bdata .gdata 

.cnf.kdata:
	$(KISSAT) $(KFLAGS) $< $*.drat | tee $@
	ls -l $*.drat | tee -a $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat

.cnf.ldata:
	$(TBSAT) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.bdata:
	$(RANDOMIZE) $< $(SEED) 
	$(TBSAT) -v $(VLEVEL) -b -i $< -p $*.order -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.gdata:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

data:
	$(INTERP) $(GRAB) "proof_added" chew-*-*.kdata > chew-kissat-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" chew-*-*.ldata > chew-tbsat-linear-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" chew-*-*.bdata > chew-tbsat-bucket-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" chew-*-*.gdata > chew-tbsat-gauss-clauses.csv
	$(INTERP) $(GRAB) -1 "process-time" chew-*-*.kdata > chew-kissat-seconds.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.ldata > chew-tbsat-linear-seconds.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.bdata > chew-tbsat-bucket-seconds.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.gdata > chew-tbsat-gauss-seconds.csv


clean:
	rm -f chew-*.cnf chew-*.schedule chew-*.order
	rm -f *~ *.lrat *.drat

superclean: clean
	rm -f *.kdata *.ldata *.bdata *.gdata

