INTERP=python3
N = 08
GDIR=../../generators
GENERATOR = $(GDIR)/chess.py
BSAT = ../../../src/bsat
TBSAT = ../../../src/tbsat
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1
RM = rm -f

chess-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r chess-$(N)

128:
	make N=128
	make chess-128.sdata RM=ls


512:
	make N=512
	make chess-512.sdata RM=ls


.SUFFIXES: .cnf .ldata .bdata .sdata .fldata .fbdata .fsdata

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

.cnf.sdata:
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.fldata:
	$(BSAT) -v $(VLEVEL) -i $< | tee $@

.cnf.fbdata:
	$(BSAT) -v $(VLEVEL) -i $< -b | tee $@

.cnf.fsdata:
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule | tee $@

clean:
	rm -f chess-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata *.fldata *.fbdata *.fsdata



