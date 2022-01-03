BSAT = ../src/tbsat
LRAT = ~/repos/drat-trim/lrat-check
DRAT = ~/repos/drat-trim/drat-trim
EXTRACT = ../../tools/xor_extractor.py

VLEVEL=1
RM=rm -f

.SUFFIXES: .cnf .ldata .bdata .sdata .gdata .lddata .bddata .sddata

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

.cnf.gdata:
	$(EXTRACT) -i $< -o $*.schedule | tee $@
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.lddata:
	$(BSAT) -v $(VLEVEL) -i $< -o $*.drat | tee $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat

.cnf.bddata:
	$(BSAT) -v $(VLEVEL) -b -i $< -o $*.drat | tee $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat

.cnf.sddata:
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.drat | tee $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat


clean:
	rm -f *.lrat *.drat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata *.lddata *.bddata *.sddata
