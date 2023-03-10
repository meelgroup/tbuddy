INTERP=python3
N = 08
TBUDDY = ~/repos/tbuddy
BDIR = $(TBUDDY)/benchmarks
GDIR= $(BDIR)/generators
GENERATOR = $(GDIR)/chess.py
TBSAT = $(TBUDDY)/tbsat/src/tbsat
CDIR = ~/repos/drat-trim
DRAT = $(CDIR)/drat-trim
LRAT = $(CDIR)/lrat-check
SDIR = ~/repos/kissat/build
KISSAT = $(SDIR)/kissat
GRAB = $(TBUDDY)/tools/grab_data.py
LINEARIZE=$(TBUDDY)/tools/linearize.py

VLEVEL=1
RM = rm -f

lchess: chess-$(N)
	make chess-$(N).ldata

bchess: chess-$(N)
	make chess-$(N).bdata

schess: chess-$(N)
	make chess-$(N).sdata

nqchess: chess-$(N)
	make chess-$(N).nqdata

kchess: chess-$(N)
	make chess-$(N).kdata

chess-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r chess-$(N)


.SUFFIXES: .cnf .ldata .bdata .sdata .kdata .nqdata

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

.cnf.nqdata:
	grep -v "q" $*.schedule > $*.nq-schedule
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.nq-schedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat


.cnf.kdata:
	$(KISSAT) $< $*.drat  | tee $@
	ls -l $*.drat | tee -a $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat

data:
	$(INTERP) $(GRAB) "Total clauses" *.ldata > chess-tbsat-linear-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.bdata > chess-tbsat-bucket-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.sdata > chess-tbsat-scan-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.nqdata > chess-tbsat-noquant-clauses.csv
	$(INTERP) $(GRAB) "proof_added" *.kdata > chess-kissat-clauses.csv
	$(INTERP) $(GRAB) "Elapsed" *.ldata > chess-tbsat-linear-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.bdata > chess-tbsat-bucket-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.sdata > chess-tbsat-scan-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.nqdata > chess-tbsat-noquant-seconds.csv
	$(INTERP) $(GRAB) -1 "process-time" *.kdata > chess-kissat-seconds.csv

	$(INTERP) $(GRAB) "verification time" *.ldata > chess-tbsat-linear-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.bdata > chess-tbsat-bucket-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.sdata > chess-tbsat-scan-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.nqdata > chess-tbsat-noquant-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.kdata > chess-kissat-check-seconds.csv


clean:
	rm -f chess-*.{cnf,schedule,nq-schedule,order}
	rm -f *.lrat *.drat *~

superclean: clean
	rm -f *.*data




