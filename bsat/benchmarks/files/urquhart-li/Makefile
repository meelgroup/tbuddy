INTERP=python3
M = 03
GDIR=../../generators
PCT=50
GENERATOR = $(GDIR)/urquhart-li-code/generate
BSAT = ../../../src/bsat
TBSAT = ../../../src/tbsat
EXTRACT = ../../../../tools/xor_extractor.py
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1
RM=rm

urquhart-li-$(M): $(GENERATOR)
	$(GENERATOR) -m$(M) -p$(PCT) -furquhart-li-$(M).cnf

.SUFFIXES: .cnf .ldata .bdata .sdata .fldata .fbdata .fsdata .xdata

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

.cnf.xdata:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.fldata:
	$(BSAT) -v $(VLEVEL) -i $< | tee $@

.cnf.fbdata:
	$(BSAT) -v $(VLEVEL) -i $< -b | tee $@



clean:
	rm -f urquhart-li-*.cnf
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata *.fldata *.fbdata *.fsdata

