BSAT = ../src/bsat-TBDD
LRAT = ~/repos/drat-trim/lrat-check

VLEVEL=1

.SUFFIXES: .cnf .ldata .bdata .sdata

.cnf.ldata:
	$(BSAT) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.bdata:
	$(BSAT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.sdata:
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

clean:
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata
