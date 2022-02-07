GEN = ../src/xtree
ROOT=xtree
N=010
SEED=123456
VLEVEL = 1
RM = rm -f

DDIR = ~/repos/drat-trim
DRAT = $(DDIR)/drat-trim

KDIR = ~/repos/kissat/build
KISSAT = $(KDIR)/kissat
KFLAGS = --unsat --no-binary

CDIR = ~/repos/cryptominisat
CMS = $(CDIR)/cryptominisat5
CFLAGS = 

INTERP=python3
TOOLDIR = ../../tools
GRAB = $(INTERP) $(TOOLDIR)/grab_data.py
PGRAB = $(INTERP) $(TOOLDIR)/grab_pair_data.py

cnf: $(ROOT)-$(N)-$(SEED).cnf

$(ROOT)-$(N)-$(SEED).cnf:
	$(GEN) -n $(N) -s $(SEED) -m n -r $(ROOT)-$(N)-$(SEED)

.SUFFIXES: .cnf .kissat_data .cms_data 

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

data:
	rm -f *.csv
	$(GRAB) "Elapsed time" *.kissat_data > kissat-generation-seconds.csv
	$(GRAB) "proof_added" *.kissat_data > kissat-clauses.csv
	$(GRAB) "verification time" *.kissat_data > kissat-drat-seconds.csv
	$(PGRAB) 1 "proof_added" 1 "verification time" *.kissat_data > kissat-drat-ratio.csv
	$(GRAB) "Total time" *.cms_data > cms-generation-seconds.csv
	$(GRAB) 2 "resolution steps" *.cms_data > cms-clauses.csv
	$(GRAB) "verification time" *.cms_data > cms-drat-seconds.csv
	$(PGRAB) 2 "resolution steps" 1 "verification time" *.cms_data > cms-drat-ratio.csv


clean:
	rm -f *~
	rm -f *.drat

superclean: clean
	rm -f *.cnf *.kissat_data *.cms_data



