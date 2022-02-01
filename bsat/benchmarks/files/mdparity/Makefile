INTERP=python3
GDIR=../../generators
GENERATOR = $(GDIR)/mdparity.py
TBSAT = ../../../src/tbsat
TDIR = ../../../../tools
EXTRACT = $(TDIR)/xor_extractor.py

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim
LRAT = $(DDIR)/lrat-check

CXDIR = ~/repos/cms-dev/cryptominisat/build
CMSX = $(CXDIR)/cryptominisat5
CXFLAGS = --presimp 1 --maxmatrixrows 50000

CDIR = ~/repos/cryptominisat
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxmatrixrows 50000

N=16
K=8
T=8
SEED=1
SOLNS=10
VLEVEL=1
RM=rm
GRAB = $(INTERP) $(GDIR)/grab_data.py


cnf: mdparity-n$(N)-k$(K)-t$(T)-s$(SEED).cnf

mdparity-n$(N)-k$(K)-t$(T)-s$(SEED).cnf: $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -k $(K) -t $(T) -s $(SEED)

.SUFFIXES: .cnf .cms_data .cmsx_data .cmsx_nocheck_data .cms_noproof_data \
	.tbsat_bucket_data .tbsat_noproof_bucket_data .tbsat_gaussian_data .tbsat_noproof_gaussian_data 

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

.cnf.tbsat_bucket_data:
	$(TBSAT) -b -v $(VLEVEL) -i $<  -o $*-tbsat.lrat | tee $@
	$(LRAT) $< $*-tbsat.lrat | tee -a $@
	ls -l $*-tbsat.lrat
	$(RM) $*-tbsat.lrat

.cnf.tbsat_noproof_bucket_data:
	$(TBSAT) -v $(VLEVEL) -b -i $< -m $(SOLNS) | tee $@

.cnf.tbsat_gaussian_data:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*-tgbsat.lrat | tee -a $@
	$(LRAT) $< $*-tgbsat.lrat | tee -a $@
	ls -l $*-tgbsat.lrat
	$(RM) $*-tgbsat.lrat

.cnf.tbsat_noproof_gaussian_data:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -m $(SOLNS) | tee -a $@

clean:
	rm -f *~
	rm -f mdparity-*.cnf mdparity-*.schedule

superclean: clean
	rm -f *.cms_noproof_data *.tbsat_noproof_bucket_data *.tbsat_noproof_gaussian_data

