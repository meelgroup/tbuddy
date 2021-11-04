INTERP=python3
N = 08
GDIR=../../generators
GENERATOR = $(GDIR)/pigeon-sinz.py

pigeon-sinz-$(N): $(GENERATOR)
	$(INTERP) $(GENERATOR) -n $(N) -r pigeon-sinz-$(N)

clean:
	rm -f pigeon-sinz-*.{cnf,schedule,order}
	rm -f *~
