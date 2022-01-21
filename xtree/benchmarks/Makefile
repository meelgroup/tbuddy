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

FDIR = ~/repos/frat
FRAT = $(FDIR)/frat-rs

MDIR = ../../tools
MATCHER = $(MDIR)/frat-matcher.py

KDIR = ~/repos/kissat/build
KISSAT = $(KDIR)/kissat
KFLAGS = --unsat --no-binary --conflicts=$(CONFLICTS)

CDIR = ~/repos/cryptominisat
CMS = $(CDIR)/cryptominisat5
CFLAGS = --maxconfl $(CONFLICTS)

CXDIR = ~/repos/cms-dev/cryptominisat/build
CMSX = $(CXDIR)/cryptominisat5
CXFLAGS = --presimp 1 --maxmatrixrows 500000

TDIR = ../../bsat/src
TBSAT = $(TDIR)/tbsat
TFLAGS = -b -v $(VLEVEL)
XEXT = $(MDIR)/xor_extractor.py

INTERP=python3
GRAB = $(INTERP) ./grab_data.py

cnf: $(ROOT)-$(N).cnf

drat: $(ROOT)-$(N).drat

frat: $(ROOT)-$(N).frat

fratc: frat $(ROOT)-$(N).frat_check_data

small: $(ROOT)-$(N).cnf  $(ROOT)-$(N).kissat_data $(ROOT)-$(N).cms_data medium

medium: $(ROOT)-$(N).drat $(ROOT)-$(N).drat_check_data medlarge

medlarge: $(ROOT)-$(N).cnf $(ROOT)-$(N).frat $(ROOT)-$(N).frat_check_data large

large: $(ROOT)-$(N).cnf $(ROOT)-$(N).cms_noproof_data $(ROOT)-$(N).tbsat_bucket_data huge

huge: $(ROOT)-$(N).cnf $(ROOT)-$(N).drat $(ROOT)-$(N).tbsat_gaussian_data $(ROOT)-$(N).tbsat_gaussian_noproof_data
# $(ROOT)-$(N).tbsat_bucket_noproof_data

fratc: $(ROOT)-$(N).cnf $(ROOT)-$(N).frat $(ROOT)-$(N).frat_check_data 


$(ROOT)-$(N).cnf:
	$(GEN) -n $(N) -s $(SEED) -m n -r $(ROOT)-$(N)

$(ROOT)-$(N).drat:
	$(GEN) -n $(N) -s $(SEED) -m d -r $(ROOT)-$(N) | tee $(ROOT)-$(N).drat_gen_data

$(ROOT)-$(N).frat:
	$(GEN) -n $(N) -s $(SEED) -m f -r $(ROOT)-$(N) | tee $(ROOT)-$(N).frat_gen_data


.SUFFIXES: .cnf .drat .frat .drat_check_data .frat_check_data .kissat_data .cms_data .cmsx_data .cms_noproof_data \
		.tbsat_bucket_data .tbsat_gaussian_data .tbsat_bucket_noproof_data .tbsat_gaussian_noproof_data


.drat.drat_check_data:
	$(DRAT) $*.cnf $< | tee $@
	ls -l $< | tee -a $@
	$(RM) $<

.frat.frat_check_data:
	{ /usr/bin/time -lp $(FRAT) elab $*.cnf $< $*.lrat 2>&1 ; } | tee $@
	 $(LRAT) $*.cnf $*.lrat	| tee -a $@
	$(RM) $<
	$(RM) $*.frat.temp
	$(RM) $*.lrat

.cnf.kissat_data:
	$(KISSAT) $(KFLAGS) $< $*-kissat.drat | tee $@
	$(DRAT) $< $*-kissat.drat | tee -a $@
	ls -l $*-kissat.drat
	$(RM) $*-kissat.drat

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

.cnf.cms_noproof_data:
	$(CMS) $(CFLAGS)  --maxmatrixrows 500000 $< | tee $@

.cnf.tbsat_bucket_data:
	$(TBSAT) $(TFLAGS) -i $<  -o $*-tbsat.lrat | tee $@
	$(LRAT) $< $*-tbsat.lrat | tee -a $@
	ls -l $*-tbsat.lrat
	$(RM) $*-tbsat.lrat

.cnf.tbsat_gaussian_data:
	$(XEXT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*-tgbsat.lrat | tee -a $@
	$(LRAT) $< $*-tgbsat.lrat | tee -a $@
	ls -l $*-tgbsat.lrat
	$(RM) $*-tgbsat.lrat

.cnf.tbsat_bucket_noproof_data:
	$(TBSAT) $(TFLAGS) -i $< | tee $@

.cnf.tbsat_gaussian_noproof_data:
	$(XEXT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule  | tee -a $@



data:
	rm -f *.csv
	$(GRAB) "Total clauses" *.drat_gen_data > tbuddy-clauses.csv
	$(GRAB) "Elapsed" *.drat_gen_data > tbuddy-generation-seconds.csv
	$(GRAB) "verification time" *.drat_check_data > tbuddy-drat-seconds.csv
	$(GRAB) "real " *.frat_check_data > tbuddy-frat-seconds.csv
	$(GRAB) "Elapsed time" *.kissat_data > kissat-generation-seconds.csv
	$(GRAB) "proof_added" *.kissat_data > kissat-clauses.csv
	$(GRAB) "verification time" *.kissat_data > kissat-drat-seconds.csv
	$(GRAB) "Total time" *.cms_data > cms-generation-seconds.csv
	$(GRAB) 2 "resolution steps" *.cms_data > cms-clauses.csv
	$(GRAB) "Total time" *.cms_noproof_data > cms-noproof-seconds.csv
	$(GRAB) "verification time" *.cms_data > cms-drat-seconds.csv
	$(GRAB) "Elapsed" *.tbsat_bucket_data > tbsat-bucket-generation-seconds.csv
	$(GRAB) "verification time" *.tbsat_bucket_data > tbsat-bucket-lrat-seconds.csv	
	$(GRAB) "Total clauses" *.tbsat_bucket_data > tbsat-bucket-clauses.csv	
	$(GRAB) "Elapsed" *.tbsat_bucket_noproof_data > tbsat-bucket-noproof-generation-seconds.csv
	$(GRAB) "Elapsed" *.tbsat_gaussian_data > tbsat-gaussian-generation-seconds.csv
	$(GRAB) "verification time" *.tbsat_gaussian_data > tbsat-gaussian-lrat-seconds.csv	
	$(GRAB) "Total clauses" *.tbsat_gaussian_data > tbsat-gaussian-clauses.csv	
	$(GRAB) "Elapsed" *.tbsat_gaussian_noproof_data > tbsat-gaussian-noproof-generation-seconds.csv

clean:
	rm -f *~
	rm -f *.drat *.frat *.lrat

superclean: clean
	rm -f *.cnf *.drat_check_data *.frat_check_data *.kissat_data *.cms_data *.cms_noproof_data 
	rm -f *.tbsat_bucket_data *.tbsat_gaussian_data *.tbsat_bucket_noproof_data *.tbsat_gaussian_noproof_data
	rm -f *.drat_gen_data *.schedule 



