CXX=g++
CFLAGS=-g -O1 -Wno-nullability-completeness
LDIR = ../../lib
LIB = $(LDIR)/buddy.a
IDIR = ../../include
INC = -I$(IDIR)

PROG = adder

all: $(PROG)

$(PROG): $(PROG).cxx $(LIB)
	$(CXX) $(CFLAGS) $(INC) -o $(PROG) $(PROG).cxx $(LIB)

clean:
	rm -f $(PROG) *.o *~
	rm -rf *.dSYM

