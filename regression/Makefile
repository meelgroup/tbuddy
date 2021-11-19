BSAT = ../src/tbsat
LRAT = ~/repos/drat-trim/lrat-check

VLEVEL=1
RM=rm -f

.SUFFIXES: .cnf .ldata .bdata .sdata

.cnf.ldata:
	$(BSAT) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.bdata:
	$(BSAT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.sdata:
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

clean:
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata
