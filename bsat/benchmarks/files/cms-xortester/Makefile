INTERP=python3
N = 10
SEED=123456
GDIR=../../generators
GENERATOR = $(GDIR)/cms-xortester/xortester.py
BSAT = ../../../src/bsat
TBSAT = ../../../src/tbsat
TDIR = ../../../../tools
EXTRACT = $(TDIR)/xor_extractor.py

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim
LRAT = $(DDIR)/lrat-check

VLEVEL=1
RM=rm
GRAB = $(INTERP) $(GDIR)/grab_data.py

CDIR = ~/repos/cryptominisat
CMS = $(CDIR)/cryptominisat5
CFLAGS = 

CXDIR = ~/repos/cms-dev/cryptominisat/build
CMSX = $(CXDIR)/cryptominisat5
CXFLAGS = --presimp 1 --maxmatrixrows 500000

cnf: xor-$(N)-$(SEED).cnf

xor-$(N)-$(SEED).cnf: $(GENERATOR)
	$(INTERP) $(GENERATOR) -s $(SEED) --varsmin $(N) > xor-$(N)-$(SEED).cnf

.SUFFIXES: .cnf .cms_data .cmsx_data .tbsat_bucket_data .tbsat_gaussian_data 

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

.cnf.tbsat_bucket_data:
	$(TBSAT) -v $(VLEVEL) -b -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.tbsat_gaussian_data:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

clean:
	rm -f xor-*.cnf
	rm -f *~ *.lrat

superclean: clean
	rm -f *.schedule *.tbsat_bucket_data *.tbsat_gaussian_data

