INTERP=python3
N = 08
GDIR=../../generators
GENERATOR = $(GDIR)/chess.py
BSAT = ../../../src/bsat
BSATT = ../../../src/bsat-TBDD
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1

pigeon-sinz-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r pigeon-sinz-$(N)

.SUFFIXES: .cnf .ldata .bdata .sdata .fsdata

.cnf.ldata:
	$(BSATT) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.bdata:
	$(BSATT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.sdata:
	$(BSATT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.fsdata:
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule | tee $@


clean:
	rm -f chess-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata

