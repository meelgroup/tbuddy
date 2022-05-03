INTERP=python3
TROOT=~/repos/tbuddy

GDIR=$(TROOT)/benchmarks/generators
GENERATOR = $(INTERP) $(GDIR)/mdparity.py
CHECKER = $(INTERP) $(GDIR)/mdpcheck.py

TBDIR=$(TROOT)/tbsat/src
TBSAT = $(TBDIR)/tbsat

TLDIR = $(TROOT)/tools
EXTRACT = $(INTERP) $(TLDIR)/xor_extractor.py

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim
LRAT = $(DDIR)/lrat-check

CXDIR = ~/repos/cms-dev/cryptominisat/build
CMSX = $(CXDIR)/cryptominisat5
CXFLAGS = --presimp 1 --maxmatrixrows 50000 --maxmatrixcols 50000

CDIR = ~/repos/cryptominisat
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxmatrixrows 50000

KDIR = ~/repos/kissat/build
KISSAT = $(KDIR)/kissat
KFLAGS = --unsat


N=8
K=2
T=2
SEED=1
SOLNS=10
VLEVEL=1
RM=rm

GRAB = $(INTERP) $(TOOLDIR)/grab_data.py

cnf: mdparity-n$(N)-k$(K)-t$(T)-s$(SEED).cnf
xcnf: mdparity-x-n$(N)-k$(K)-t$(T)-s$(SEED).cnf

mdparity-n$(N)-k$(K)-t$(T)-s$(SEED).cnf:
	$(GENERATOR) -n $(N) -k $(K) -t $(T) -s $(SEED) -p

mdparity-x-n$(N)-k$(K)-t$(T)-s$(SEED).cnf:
	$(GENERATOR) -x -n $(N) -k $(K) -t $(T) -s $(SEED) -p

.SUFFIXES: .cnf .cms_data .cmsx_data .cmsx_nocheck_data .cms_noproof_data \
	.tbsat_bucket_data .tbsat_noproof_bucket_data .tbsat_gaussian_data .tbsat_noproof_gaussian_data \
	.kissat_data

.cnf.cms_data:
	$(CMS) $(CFLAGS)  $< $*-cms.drat | tee $@
	$(DRAT) $< $*-cms.drat | tee -a $@
	ls -l $*-cms.drat
	$(RM) $*-cms.drat 

.cnf.cmsx_data:
	$(CMSX) $(CXFLAGS)  $< $*-cmsx.drat | tee $@
	$(DRAT) $< $*-cmsx.drat | tee -a $@
	ls -l $*-cmsx.drat
	$(RM) $*-cmsx.drat 

.cnf.cmsx_nocheck_data:
	$(CMSX) $(CXFLAGS)  $< $*-cmsx.drat | tee $@
	ls -l $*-cmsx.drat

.cnf.cms_noproof_data:
	$(CMS) $(CFLAGS)  $< | tee $@
	$(CHECKER) $< $@ | tee -a $@

.cnf.kissat_data:
	$(KISSAT) $(KFLAGS)  $< $*-kissat.drat | tee $@
	$(DRAT) $< $*-kissat.drat | tee -a $@
	ls -l $*-kissat.drat
	$(RM) $*-kissat.drat 

.cnf.tbsat_bucket_data:
	$(TBSAT) -b -v $(VLEVEL) -i $< -p $*.order -o $*-tbsat.lrat | tee $@
	$(LRAT) $< $*-tbsat.lrat | tee -a $@
	ls -l $*-tbsat.lrat
	$(RM) $*-tbsat.lrat

.cnf.tbsat_noproof_bucket_data:
	$(TBSAT) -v $(VLEVEL) -b -i $< -p $*.order -m $(SOLNS) | tee $@
	$(CHECKER) $< $@ | tee -a $@

.cnf.tbsat_gaussian_data:
	$(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -p $*.order -o $*-tgbsat.lrat | tee -a $@
	$(LRAT) $< $*-tgbsat.lrat | tee -a $@
	ls -l $*-tgbsat.lrat
	$(RM) $*-tgbsat.lrat

.cnf.tbsat_noproof_gaussian_data:
	$(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -p $*.order -m $(SOLNS) | tee -a $@
	$(CHECKER) $< $@ | tee -a $@

clean:
	rm -f *~
	rm -f *.cnf *.schedule *.lrat *.order

superclean: clean
	rm -f *.*data

data:
	$(GRAB) "Total time" *.cms_noproof_data > cms-noproof-seconds.csv
