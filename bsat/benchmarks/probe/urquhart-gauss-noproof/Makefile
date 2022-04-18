INTERP=python3
SIZE = 08
VLEVEL = 01
GDIR=../../generators
GENERATOR = $(GDIR)/urquhart-li-code/generate
PCT=50
SDIR = ../../../src
SOLVER = $(SDIR)/tbsat
TDIR= ~/repos/tbuddy/tools
EXTRACT=$(TDIR)/xor_extractor.py
TLIM=10
ROOT=urquhart-gauss-noproof

grun: gen run clear

gen:
	$(GENERATOR) -m$(SIZE) -p$(PCT) -f$(ROOT)-$(SIZE).cnf
	$(INTERP) $(EXTRACT) -i $(ROOT)-$(SIZE).cnf -o $(ROOT)-$(SIZE).schedule | tee $(ROOT)-$(SIZE).data

run:
	$(SOLVER) -v $(VLEVEL) -t $(TLIM) -i $(ROOT)-$(SIZE).cnf -s $(ROOT)-$(SIZE).schedule | tee -a $(ROOT)-$(SIZE).data

clear:
	rm -f $(ROOT)-$(SIZE).{order,schedule,cnf}

clean:
	rm -f $(ROOT)-*.{cnf,schedule,order}
	rm -f *~

superclean: clean
	rm -f $(ROOT)-*.data




