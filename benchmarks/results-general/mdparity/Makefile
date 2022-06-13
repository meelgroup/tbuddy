INTERP=python3
TROOT=../../..
TLIM=5000

GDIR=$(TROOT)/benchmarks/generators
GENERATOR = $(INTERP) $(GDIR)/mdparity.py
CHECKER = $(INTERP) $(GDIR)/mdpcheck.py

TBDIR=$(TROOT)/tbsat/src
TBSAT = $(TBDIR)/tbsat

TLDIR = $(TROOT)/tools
EXTRACT = $(INTERP) $(TLDIR)/xor_extractor.py
GRAB = $(INTERP) $(TOOLDIR)/grab_data.py

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim
LRAT = $(DDIR)/lrat-check

FDIR = ~/repos/frat
FRAT = $(FDIR)/frat-rs

CXDIR = ~/repos/cms-dev/cryptominisat/build
CMSX = $(CXDIR)/cryptominisat5
CXFLAGS = --presimp 1 --maxmatrixrows 10000 --maxmatrixcols 10000

CDIR = ~/repos/cryptominisat/build
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxmatrixrows 50000

KDIR = ~/repos/kissat/build
KISSAT = $(KDIR)/kissat
KFLAGS = --unsat --seconds $(TLIM)


N=8
M=16
Y=125
SEED=1
SOLNS=10
VLEVEL=1
RM=rm

tsrun: mdp-sat-n$(N)-m$(M)-Y$(Y)-s$(SEED).tbsat_sat_data

tprun: mdp-nosat-n$(N)-m$(M)-Y$(Y)-s$(SEED).tbsat_proof_data

turun: mdp-unique-n$(N)-m$(M)-Y$(Y)-s$(SEED).tbsat_proof_data

cxprun: mdp-nosat-n$(N)-m$(M)-Y$(Y)-s$(SEED).cmsx_proof_data

cxurun: mdp-unique-n$(N)-m$(M)-Y$(Y)-s$(SEED).cmsx_proof_data

mdp-nosat-n$(N)-m$(M)-Y$(Y)-s$(SEED).cnf:
	$(GENERATOR) -r mdp-nosat-n$(N)-m$(M)-Y$(Y)-s$(SEED) -p -n $(N) -m $(M) -y 0.$(Y) -t -1 -s $(SEED)

mdp-sat-n$(N)-m$(M)-Y$(Y)-s$(SEED).cnf:
	$(GENERATOR) -r mdp-sat-n$(N)-m$(M)-Y$(Y)-s$(SEED) -p -n $(N) -m $(M) -y 0.$(Y) -t 0 -s $(SEED)

mdp-unique-n$(N)-m$(M)-Y$(Y)-s$(SEED).cnf:
	$(GENERATOR) -r mdp-unique-n$(N)-m$(M)-Y$(Y)-s$(SEED) -p -x -n $(N) -m $(M) -y 0.$(Y) -t 0 -s $(SEED)

.SUFFIXES: .cnf .cms_proof_data .cms_noproof_data .cms_sat_data \
	.cmsx_proof_data .cmsx_sat_data \
	.kissat_proof_data .kissat_noproof_data .kissat_sat_data \
	.tbsat_proof_data .tbsat_noproof_data .tbsat_sat_data 

.cnf.cms_proof_data:
	$(CMS) $(CFLAGS)  $< $*-cms.drat | tee $@
	$(DRAT) $< $*-cms.drat | tee -a $@
	ls -l $*-cms.drat | tee -a $@
	$(RM) $*-cms.drat 

.cnf.cms_noproof_data:
	$(CMS) $(CFLAGS)  $< | tee $@

.cnf.cms_sat_data:
	$(CMS) $(CFLAGS)  $< | tee $@
	$(CHECKER) $< $@ | tee -a $@

.cnf.cmsx_proof_data:
	$(CMSX) $(CXFLAGS)  $< $*-cmsx.frat | tee $@
	{ /usr/bin/time -lp $(FRAT) elab $< $*-cmsx.frat $*.lrat 2>&1 ; } | tee -a $@
	$(LRAT) $*.cnf $*.lrat	| tee -a $@
	$(RM) $*-cmsx.frat
	$(RM) $*-cmsx.frat.temp
	$(RM) $*.lrat

.cnf.cmsx_noproof_data:
	$(CMSX) $(CFLAGS)  $< | tee $@

.cnf.cmsx_sat_data:
	$(CMSX) $(CFLAGS)  $< | tee $@
	$(CHECKER) $< $@ | tee -a $@

.cnf.kissat_proof_data:
	$(KISSAT) $(KFLAGS)  $< $*-kissat.drat | tee $@
	$(DRAT) $< $*-kissat.drat | tee -a $@
	ls -l $*-kissat.drat | tee -a $@
	$(RM) $*-kissat.drat 

.cnf.kissat_noproof_data:
	$(KISSAT) $(KFLAGS) $< | tee $@

.cnf.kissat_sat_data:
	$(KISSAT) $(KFLAGS) $< | tee $@
	$(CHECKER) $< $@ | tee -a $@

.cnf.tbsat_proof_data:
	$(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -p $*.order -o $*-tbsat.lrat | tee -a $@
	$(LRAT) $< $*-tbsat.lrat | tee -a $@
	ls -l $*-tbsat.lrat | tee -a $@
	$(RM) $*-tbsat.lrat

.cnf.tbsat_noproof_data:
	$(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -p $*.order -m $(SOLNS) | tee -a $@

.cnf.tbsat_sat_data:
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
