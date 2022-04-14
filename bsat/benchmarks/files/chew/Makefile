INTERP=python3
N = 10
SEED=123456
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
BSAT = ../../../src/bsat
TBSAT = ../../../src/tbsat
TOOLDIR = ../../../../tools
EXTRACT = $(TOOLDIR)/xor_extractor.py
RANDOMIZE = $(TOOLDIR)/randomizer.py
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1
RM=rm

cnf: chew-$(N)-$(SEED).cnf

bdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).bdata

rbdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).rbdata

xdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).xdata

chew-$(N)-$(SEED).cnf: $(GENERATOR)
	$(GENERATOR) $(N) $(SEED) 
	mv formula.cnf chew-$(N)-$(SEED).cnf

.SUFFIXES: .cnf .ldata .bdata .xdata .rbdata

.cnf.ldata:
	$(TBSAT) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.bdata:
	$(TBSAT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.rbdata:
	$(RANDOMIZE) $< $(SEED) 
	$(TBSAT) -v $(VLEVEL) -b -i $< -p $*.order -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.xdata:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat




clean:
	rm -f chew-*.cnf
	rm -f *~ *.lrat

superclean: clean
	rm -f *.ldata *.bdata *.xdata *.rbdata

