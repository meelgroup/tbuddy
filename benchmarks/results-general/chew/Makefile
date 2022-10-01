INTERP=python3
N = 10
SEED=1
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf
KDIR=~/repos/kissat/build
KISSAT=$(KDIR)/kissat
TLIM=900
KFLAGS = --unsat --no-binary --time=$(TLIM)
TBSAT = ../../../tbsat/src/tbsat
TOOLDIR = ../../../tools
EXTRACT = $(TOOLDIR)/xor_extractor.py
GRAB = $(TOOLDIR)/grab_data.py
RANDOMIZE = $(TOOLDIR)/randomizer.py
DDIR=~/repos/drat-trim
DRAT= $(DDIR)/drat-trim
PDIR=~/repos/pgbdd
PGBDD=$(PDIR)/prototype/solver.py
PEXTRACT=$(PDIR)/extractor/xor_extractor.py
LRAT = $(DDIR)/lrat-check
VLEVEL=1
RM=rm
TLIM=600

cnf: chew-$(N)-$(SEED).cnf

kdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).kdata

ddata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).ddata

bdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).bdata

gdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).gdata

nbdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).nbdata

ngdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).ngdata

pbdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).pbdata

pgdata: chew-$(N)-$(SEED).cnf chew-$(N)-$(SEED).pgdata

chew-$(N)-$(SEED).cnf: $(GENERATOR)
	$(GENERATOR) $(N) $(SEED) 
	mv formula.cnf chew-$(N)-$(SEED).cnf

.SUFFIXES: .cnf .kdata .ddata .bdata .gdata .pbdata .pgdata .nbdata .ngdata

.cnf.kdata:
	$(KISSAT) $(KFLAGS) $< $*.drat | tee $@
	ls -l $*.drat | tee -a $@
	$(DRAT) $< $*.drat | tee -a $@
	$(RM) $*.drat

.cnf.ddata:
	$(TBSAT) -v $(VLEVEL) -t $(TLIM) -i $< -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.bdata:
	$(RANDOMIZE) $< $(SEED) 
	$(TBSAT) -v $(VLEVEL) -b -i $< -p $*.order -o $*.lrat | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.gdata:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule -o $*.lrat | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.nbdata:
	$(RANDOMIZE) $< $(SEED) 
	$(TBSAT) -v $(VLEVEL) -b -i $< -p $*.order | tee $@

.cnf.ngdata:
	$(INTERP) $(EXTRACT) -i $< -o $*.schedule | tee $@
	$(TBSAT) -v $(VLEVEL) -i $< -s $*.schedule | tee -a $@


.cnf.pbdata:
	$(RANDOMIZE) $< $(SEED) 
	$(INTERP) $(PGBDD) -v $(VLEVEL) -b -t $(TLIM) -i $< -p $*.order -o $*.lrat 2>&1 | tee $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

.cnf.pgdata:
	$(INTERP) $(PEXTRACT) -i $< -o $*.schedule | tee $@
	$(INTERP) $(PGBDD) -v $(VLEVEL) -t $(TLIM) -i $< -s $*.schedule -o $*.lrat 2>&1 | tee -a $@
	ls -l $*.lrat | tee -a $@
	$(LRAT) $< $*.lrat | tee -a $@
	$(RM) $*.lrat

data:
	$(INTERP) $(GRAB) "proof_added" chew-*-*.kdata > chew-kissat-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" chew-*-*.ddata > chew-tbsat-direct-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" chew-*-*.bdata > chew-tbsat-bucket-clauses.csv
	$(INTERP) $(GRAB) "Total clauses" chew-*-*.gdata > chew-tbsat-gauss-clauses.csv
	$(INTERP) $(GRAB) "Total BDD nodes produced" chew-*-*.bdata > chew-tbsat-bucket-nodes.csv
	$(INTERP) $(GRAB) -1 "process-time" chew-*-*.kdata > chew-kissat-seconds.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.ddata > chew-tbsat-direct-seconds.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.bdata > chew-tbsat-bucket-seconds.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.gdata > chew-tbsat-gauss-seconds.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.nbdata > chew-tbsat-bucket-noproof-seconds.csv
	$(INTERP) $(GRAB) "Total BDD nodes produced" chew-*-*.nbdata > chew-tbsat-bucket-noproof-nodes.csv
	$(INTERP) $(GRAB) "Elapsed seconds" chew-*-*.ngdata > chew-tbsat-gauss-noproof-seconds.csv
	$(INTERP) $(GRAB) "Elapsed time" chew-*-*.pgdata > chew-pgpbs-seconds.csv
	$(INTERP) $(GRAB) "Elapsed time" chew-*-*.pbdata > chew-pgbdd-seconds.csv
	$(INTERP) $(GRAB) "verification time" chew-*-*.bdata > chew-tbsat-bucket-check.csv
	$(INTERP) $(GRAB) "verification time" chew-*-*.gdata > chew-tbsat-gauss-check.csv
	$(INTERP) $(GRAB) "verification time" chew-*-*.kdata > chew-kissat-check.csv

clean:
	rm -f chew-*.cnf chew-*.schedule chew-*.order
	rm -f *~ *.lrat *.drat

superclean: clean
	rm -f *.kdata *.ddata *.bdata *.gdata

