INTERP=python3
N = 08
GDIR=../../generators
GENERATOR = $(GDIR)/chess.py
TBSAT = ../../../tbsat/src/tbsat
LRAT = ~/repos/drat-trim/lrat-check
CHECK = ../../../src/sat_check
GRAB = ../../../tools/grab_data.py
SOLNS=10
VLEVEL=1
RM = rm -f

schess: chess-$(N)
	make chess-$(N).sdata

chess-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r chess-$(N)

chess-sat-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -c -n $(N) -r chess-sat-$(N)


.SUFFIXES: .cnf .ldata .bdata .bsdata .sdata .ssdata .fldata .fbdata .fsdata

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
	$(TBSAT) -v $(VLEVEL) -b -i $< | tee $*.solve  tee $@ 
	$(CHECK) -v $(VLEVEL) -i $< -s $*.solve | tee -a $@

.cnf.sdata:
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.ssdata:
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -m $(SOLNS) | tee $*.solve  tee $@ 
	$(CHECK) -v $(VLEVEL) -i $< -s $*.solve | tee -a $@


.cnf.fldata:
	$(BSAT) -v $(VLEVEL) -i $< | tee $@

.cnf.fbdata:
	$(BSAT) -v $(VLEVEL) -i $< -b | tee $@

.cnf.fsdata:
	$(BSAT) -v $(VLEVEL) -i $< -s $*.schedule | tee $@

data:
	$(INTERP) $(GRAB) "Total clauses" *.bdata > chess-tbsat-bucket-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.sdata > chess-tbsat-scan-clauses.csv
	$(INTERP) $(GRAB) "Elapsed" *.bdata > chess-tbsat-bucket-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.sdata > chess-tbsat-scan-seconds.csv

clean:
	rm -f chess-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata *.fldata *.fbdata *.fsdata



