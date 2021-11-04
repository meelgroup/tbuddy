INTERP=python3
N = 08
GDIR=../../generators
GENERATOR = $(GDIR)/chess.py

chess-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r chess-$(N)

clean:
	rm -f chess-*.{cnf,schedule,order}
	rm -f *~
