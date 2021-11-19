INTERP=python3
M = 03
GDIR=../../generators
PCT=50
GENERATOR = $(GDIR)/urquhart-li-code/generate
BSAT = ../../../src/bsat
TBSAT = ../../../src/tbsat
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1

urquhart-li-$(M): $(GENERATOR)
	$(GENERATOR) -m$(M) -p$(PCT) -furquhart-li-$(M).cnf

.SUFFIXES: .cnf .ldata .bdata .sdata .fldata .fbdata .fsdata

.cnf.ldata:
	$(TBSAT) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.bdata:
	$(TBSAT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.fldata:
	$(BSAT) -v $(VLEVEL) -i $< | tee $@

.cnf.fbdata:
	$(BSAT) -v $(VLEVEL) -i $< -b | tee $@



clean:
	rm -f urquhart-li-*.cnf
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata *.fldata *.fbdata *.fsdata

