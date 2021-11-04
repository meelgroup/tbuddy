N = 10
SEED=123456
GDIR=../../generators
GENERATOR = $(GDIR)/chew-code/parity-cnf

chew-$(N)-$(SEED).cnf: $(GENERATOR)
	$(GENERATOR) $(N) $(SEED) 
	mv formula.cnf chew-$(N)-$(SEED).cnf

clean:
	rm -f chew-*.cnf
	rm -f *~
