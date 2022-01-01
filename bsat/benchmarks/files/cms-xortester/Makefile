INTERP=python3
N = 10
SEED=123456
GDIR=../../generators
GENERATOR = $(GDIR)/cms-xortester/xortester.py
BSAT = ../../../src/bsat
TBSAT = ../../../src/tbsat
TDIR = ../../../../tools
EXTRACT = $(TDIR)/xor_extractor.py
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1
RM=rm

xor-$(N)-$(SEED).cnf: $(GENERATOR)
	$(INTERP) $(GENERATOR) -s $(SEED) --varsmin $(N) > xor-$(N)-$(SEED).cnf

.SUFFIXES: .cnf .tbdata .tgdata

.cnf.tbdata:
	$(TBSAT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.tgdata:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

clean:
	rm -f xor-*.cnf
	rm -f *~ *.lrat

superclean: clean
	rm -f *.schedule *.tbdata *.tgdata

