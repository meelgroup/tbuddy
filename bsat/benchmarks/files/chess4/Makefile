INTERP=python3
N = 06
GDIR=../../generators
GENERATOR = $(GDIR)/chess4.py
TBSAT = ../../../src/tbsat
LRAT = ~/repos/drat-trim/lrat-check
CHECK = ../../../src/sat_check
VLEVEL=1
RM = rm -f

EXDIR = ~/repos/pgbdd/extractor
XHACK = ./xhack.py
EXTRACT = $(EXDIR)/constraint_extractor.py
PGDIR = ~/repos/pgbdd/prototype
PGPBS = $(PGDIR)/solver.py



chess4-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r chess4-$(N)

.SUFFIXES: .cnf .ldata .bdata .bsdata .sdata .ssdata .edata .tedata

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

.cnf.bsdata:
	$(TBSAT) -v $(VLEVEL) -b -i $< | tee $*.solve | tee $@ 
	$(CHECK) -v $(VLEVEL) -i $< -s $*.solve | tee -a $@

.cnf.sdata:
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.ssdata:
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule | tee $*.solve | tee $@ 
	$(CHECK) -v $(VLEVEL) -i $< -s $*.solve | tee -a $@

.cnf.edata:
	$(INTERP) $(EXTRACT) -v $(VLEVEL) -i $< -o $*.cschedule | tee $@
	$(INTERP) $(PGPBS) -i $< -s $*.cschedule -o $*.lrat 2>&1 | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.tedata:
	$(INTERP) $(EXTRACT) -v $(VLEVEL) -i $< -o $*.cschedule | tee $@
	$(INTERP) $(XHACK) < $*.cschedule > $*.xschedule
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.xschedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

clean:
	rm -f chess4-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata *.bsdata *.ssdata .edata

