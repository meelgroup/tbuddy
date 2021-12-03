CXX=g++
CFLAGS=-g -O1 -Wno-nullability-completeness
LDIR = ../../lib
LIB = $(LDIR)/tbuddy.a
IDIR = ../../include
INC = -I$(IDIR)

PROG = xor

all: $(PROG)

$(PROG): $(PROG).cxx $(LIB)
	$(CXX) $(CFLAGS) $(INC) -static -o $(PROG) $(PROG).cxx $(LIB)

clean:
	rm -f $(PROG) *.o *~
	rm -rf *.dSYM

