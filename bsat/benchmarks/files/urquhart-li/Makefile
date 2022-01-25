INTERP=python3
M = 03
GDIR=../../generators
PCT=50
GENERATOR = $(GDIR)/urquhart-li-code/generate
BSAT = ../../../src/bsat
TBSAT = ../../../src/tbsat
EXTRACT = ../../../../tools/xor_extractor.py
LRAT = ~/repos/drat-trim/lrat-check
VLEVEL=1
RM=rm
GRAB = $(INTERP) $(GDIR)/grab_data.py

cnf: urquhart-li-$(M).cnf

urquhart-li-$(M).cnf: $(GENERATOR)
	$(GENERATOR) -m$(M) -p$(PCT) -furquhart-li-$(M).cnf


large: urquhart-li-$(M).cnf urquhart-li-$(M).tbsat_bucket_data urquhart-li-$(M).tbsat_bucket_noproof_data huge

huge: urquhart-li-$(M).cnf urquhart-li-$(M).tbsat_gaussian_data urquhart-li-$(M).tbsat_gaussian_noproof_data




.SUFFIXES: .cnf .tbsat_bucket_data .tbsat_gaussian_data .tbsat_bucket_noproof_data .tbsat_gaussian_noproof_data


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

.cnf.tbsat_bucket_noproof_data:
	$(TBSAT) -v $(VLEVEL) -b -i $< | tee $@

.cnf.tbsat_gaussian_noproof_data:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule | tee -a $@

data:
	rm -f *.csv
	$(GRAB) "Elapsed" *.tbsat_bucket_data > tbsat-bucket-generation-seconds.csv
	$(GRAB) "verification time" *.tbsat_bucket_data > tbsat-bucket-lrat-seconds.csv	
	$(GRAB) "Total clauses" *.tbsat_bucket_data > tbsat-bucket-clauses.csv	
	$(GRAB) "Elapsed" *.tbsat_bucket_noproof_data > tbsat-bucket-noproof-generation-seconds.csv
	$(GRAB) "Elapsed" *.tbsat_gaussian_data > tbsat-gaussian-generation-seconds.csv
	$(GRAB) "verification time" *.tbsat_gaussian_data > tbsat-gaussian-lrat-seconds.csv	
	$(GRAB) "Total clauses" *.tbsat_gaussian_data > tbsat-gaussian-clauses.csv	
	$(GRAB) "Elapsed" *.tbsat_gaussian_noproof_data > tbsat-gaussian-noproof-generation-seconds.csv


clean:
	rm -f urquhart-li-*.cnf
	rm -f *.lrat *~ *.schedule

superclean: clean
	rm -f *data

