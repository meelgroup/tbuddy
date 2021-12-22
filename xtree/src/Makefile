CXX=g++
OPT=-O2
#OPT=-O0
CFLAGS=-g $(OPT) -Wno-nullability-completeness
INC=-I../../buddy/include
LDIR = ../..//buddy/lib
TLIB = $(LDIR)/tbuddy.a
VERB=1

all: xtree xtreeb

xtree: xtree.cpp
	$(CXX) $(CFLAGS) $(INC) -DVERBOSITY=$(VERB) -o xtree xtree.cpp $(TLIB)

xtreeb: xtree.cpp
	$(CXX) $(CFLAGS) $(INC) -DBINARY -DVERBOSITY=$(VERB) -o xtreeb xtree.cpp $(TLIB)


clean:
	rm -f *.o *~
	rm -f *.cnf *.drat *.dratb
	rm -f xtree xtreeb

