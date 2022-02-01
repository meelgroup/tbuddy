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
CXFLAGS = --presimp 1 --maxmatrixrows 500000

CDIR = ~/repos/cryptominisat
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxmatrixrows 50000

N=16
K=8
T=8
SEED=1
SOLNS=10


VLEVEL=2
RM=rm
GRAB = $(INTERP) $(GDIR)/grab_data.py



cnf: mdparity-n$(N)-k$(K)-t$(T)-s$(SEED).cnf

mdparity-n$(N)-k$(K)-t$(T)-s$(SEED).cnf: $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -k $(K) -t $(T) -s $(SEED)

.SUFFIXES: .cnf .cms_noproof_data .tbsat_noproof_bucket_data .tbsat_noproof_gaussian_data 

.cnf.cms_noproof_data:
	$(CMS) $(CFLAGS)  $< | tee $@

.cnf.tbsat_noproof_bucket_data:
	$(TBSAT) -v $(VLEVEL) -b -i $< -m $(SOLNS) | tee $@

.cnf.tbsat_noproof_gaussian_data:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -m $(SOLNS) | tee -a $@

clean:
	rm -f *~
	rm -f mdparity-*.cnf mdparity-*.schedule

superclean: clean
	rm -f *.cms_noproof_data *.tbsat_noproof_bucket_data *.tbsat_noproof_gaussian_data

