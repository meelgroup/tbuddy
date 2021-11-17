INTERP=python3
N = 08
GDIR=../../generators
GENERATOR = $(GDIR)/chess.py
BSAT = ../../../src/bsat-TBDD
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1


chess-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r chess-$(N)

.SUFFIXES: .cnf .ldata .bdata .sdata

.cnf.ldata:
	$(BSAT) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.bdata:
	$(BSAT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

.cnf.sdata:
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	rm $*.lrat

clean:
	rm -f chess-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata


