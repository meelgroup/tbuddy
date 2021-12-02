GEN = ../src/xtree
ROOT=xtree
N=010
SEED=123456
CONFLICTS=100000000

RM = rm -f

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim

KDIR = ~/repos/kissat/build
KISSAT = $(KDIR)/kissat
KFLAGS = --unsat --no-binary --conflicts=$(CONFLICTS)

CDIR = ~/repos/cryptominisat/
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxconfl $(CONFLICTS)

$(ROOT)-$(N).cnf:
	$(GEN) $(N) $(SEED) | tee $(ROOT)-$(N).data

.SUFFIXES: .cnf .xdata .kdata .cdata .fcdata

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

.cnf.fcdata:
	$(CMS) $(CFLAGS)  $< | tee $@

clean:
	rm -f *~
	rm -f *.cnf *.drat

superclean: clean
	rm -f *.xdata *.kdata *.cdata
