INTERP=python3
GDIR=../../generators
GENERATOR = $(GDIR)/pigeon-sinz.py
TBUDDY = ~/repos/tbuddy
BDIR = $(TBUDDY)/benchmarks
TBSAT = $(TBUDDY)/tbsat/src/tbsat
CDIR = ~/repos/drat-trim
DRAT = $(CDIR)/drat-trim
LRAT = $(CDIR)/lrat-check
SDIR = ~/repos/kissat/build
KISSAT = $(SDIR)/kissat
KFLAGS = --unsat --no-binary
GRAB = $(TBUDDY)/tools/grab_data.py
LRAT = ~/repos/drat-trim/lrat-check

VLEVEL=1
N = 08
RM = rm
TLIM=1000

lpigeon: pigeon-sinz-$(N).cnf pigeon-sinz-$(N).ldata

bpigeon: pigeon-sinz-$(N).cnf pigeon-sinz-$(N).bdata

spigeon: pigeon-sinz-$(N).cnf pigeon-sinz-$(N).sdata

kpigeon: pigeon-sinz-$(N).cnf pigeon-sinz-$(N).kdata


pigeon-sinz-$(N).cnf: $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r pigeon-sinz-$(N)

.SUFFIXES: .cnf .ldata .bdata .sdata .kdata

.cnf.ldata:
	$(TBSAT) -t $(TLIM) -v $(VLEVEL) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.bdata:
	$(TBSAT) -t $(TLIM) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.sdata:
	$(TBSAT) -t $(TLIM) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.kdata:
	$(KISSAT) $(KFLAGS) --time=$(TLIM) $< $*.drat  | tee $@
	ls -l $*.drat | tee -a $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat

data:

	$(INTERP) $(GRAB) "Total clauses" *.ldata > pigeon-sinz-tbsat-linear-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.bdata > pigeon-sinz-tbsat-bucket-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" *.sdata > pigeon-sinz-tbsat-scan-clauses.csv
	$(INTERP) $(GRAB) "proof_added" *.kdata > pigeon-sinz-kissat-clauses.csv
	$(INTERP) $(GRAB) "Elapsed" *.ldata > pigeon-sinz-tbsat-linear-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.bdata > pigeon-sinz-tbsat-bucket-seconds.csv
	$(INTERP) $(GRAB) "Elapsed" *.sdata > pigeon-sinz-tbsat-scan-seconds.csv
	$(INTERP) $(GRAB) -1 "process-time" *.kdata > pigeon-sinz-kissat-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.ldata > pigeon-sinz-tbsat-linear-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.bdata > pigeon-sinz-tbsat-bucket-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.sdata > pigeon-sinz-tbsat-scan-check-seconds.csv
	$(INTERP) $(GRAB) "verification time" *.kdata > pigeon-sinz-kissat-check-seconds.csv


clean:
	rm -f chess-*.{cnf,schedule,order}
	rm -f *.lrat *~

superclean: clean
	rm -f *.ldata *.bdata *.sdata *.kdata


