GEN = ../src/xtree
ROOT=xtree
N=010
SEED=123456
CONFLICTS=1000000000
VLEVEL = 1

RM = rm -f

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim
LRAT = $(DDIR)/lrat-check

KDIR = ~/repos/kissat/build
KISSAT = $(KDIR)/kissat
KFLAGS = --unsat --no-binary --conflicts=$(CONFLICTS)

CDIR = ~/repos/cryptominisat/
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxconfl $(CONFLICTS)

TDIR = ../../bsat/src/
TBSAT = $(TDIR)/tbsat
TFLAGS = -b -v $(VLEVEL)

INTERP=python3
GRAB = $(INTERP) ./grab_data.py

$(ROOT)-$(N).cnf:
	$(GEN) $(N) $(SEED) | tee $(ROOT)-$(N).data

.SUFFIXES: .cnf .xdata .kdata .cdata .fcdata .tdata

.cnf.xdata:
	$(DRAT) $< $*.drat | tee $@
	ls -l $*.drat | tee -a $@
	$(RM) $*.drat

.cnf.kdata:
	$(KISSAT) $(KFLAGS) $< $*-kissat.drat | tee $@
	$(DRAT) $< $*-kissat.drat | tee -a $@
	ls -l $*-kissat.drat
	$(RM) $*-kissat.drat

.cnf.cdata:
	$(CMS) $(CFLAGS)  $< $*-cms.drat | tee $@
	$(DRAT) $< $*-cms.drat | tee -a $@
	ls -l $*-cms.drat
	$(RM) $*-cms.drat 

.cnf.tdata:
	$(TBSAT) $(TFLAGS) -i $<  -o $*-tbsat.lrat | tee $@
	$(LRAT) $< $*-tbsat.lrat | tee -a $@
	ls -l $*-tbsat.lrat
	$(RM) $*-tbsat.lrat

.cnf.fcdata:
	$(CMS) $(CFLAGS)  $< | tee $@

tbuddy-data:
	rm -f tbuddy-*.csv
	$(GRAB) "Total clauses" *.data > tbuddy-clauses.csv
	$(GRAB) "Elapsed" *.data > tbuddy-generation-seconds.csv
	$(GRAB) "verification time" *.xdata > tbuddy-drat-seconds.csv


data:
	rm -f *.csv
	$(GRAB) "Total clauses" *.data > tbuddy-clauses.csv
	$(GRAB) "Elapsed" *.data > tbuddy-generation-seconds.csv
	$(GRAB) "verification time" *.xdata > tbuddy-drat-seconds.csv
	$(GRAB) "Elapsed time" *.kdata > kissat-generation-seconds.csv
	$(GRAB) "proof_added" *.kdata > kissat-clauses.csv
	$(GRAB) "Total time" *.cdata > cms-generation-seconds.csv
	$(GRAB) "Total time" *.fcdata > cms-noproof-seconds.csv
	$(GRAB) "Elapsed" *.tdata > tbsat-generation-seconds.csv
	$(GRAB) "verification time" *.tdata > tbsat-lrat-seconds.csv	
	$(GRAB) "Total clauses" *.tdata > tbsat-clauses.csv	

clean:
	rm -f *~
	rm -f *.cnf *.drat

superclean: clean
	rm -f *.data *.xdata *.kdata *.cdata

