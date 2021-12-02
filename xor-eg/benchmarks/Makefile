GEN = ../src/xtree
ROOT=xtree
N=010
SEED=123456
CONFLICTS=1000000

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim

KDIR = ~/repos/kissat/build
KISSAT = $(KDIR)/kissat
KFLAGS = --unsat --no-binary --conflicts=$(CONFLICTS)

CDIR = ~/repos/cryptominisat/
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxconfl $(CONFLICTS)

$(ROOT)-$(N).cnf:
	$(GEN) $(N) $(SEED) | tee $(ROOT)-$(N).xdata
	$(DRAT) $(ROOT)-$(N).cnf $(ROOT)-$(N).drat | tee -a $(ROOT)-$(N).xdata

.SUFFIXES: .cnf .xdata .kdata .cdata .fcdata

.cnf.kdata:
	$(KISSAT) $(KFLAGS) $< $*-kissat.drat | tee $@
	$(DRAT) $< $*-kissat.drat | tee -a $@

.cnf.cdata:
	$(CMS) $(CFLAGS)  $< $*-cms.drat | tee $@
	$(DRAT) $< $*-cms.drat | tee -a $@

.cnf.fcdata:
	$(CMS) $(CFLAGS)  $< | tee $@

clean:
	rm -f *~
	rm -f *.cnf *.drat

superclean: clean
	rm -f *.xdata *.kdata *.cdata
