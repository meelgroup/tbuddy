M = 03
GDIR=../../generators
PCT=50
GENERATOR = $(GDIR)/urquhart-li-code/generate

urquhart-li-$(M): $(GENERATOR)
	$(GENERATOR) -m$(M) -p$(PCT) -furquhart-li-$(M).cnf

clean:
	rm -f urquhart-li-*.cnf
	rm -f *~
