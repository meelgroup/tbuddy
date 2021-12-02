CXX=g++
##OPT=-O2
OPT=-O0
CFLAGS=-g $(OPT) -Wno-nullability-completeness
INC=-I../../buddy/include
LDIR = ../..//buddy/lib
TLIB = $(LDIR)/tbuddy.a

all: xtree

xstrip: xstrip.cpp
	$(CXX) $(CFLAGS) $(INC) -o xstrip xstrip.cpp $(TLIB)

xtree: xtree.cpp
	$(CXX) $(CFLAGS) $(INC) -o xtree xtree.cpp $(TLIB)


clean:
	rm -f *.o *~
	rm -f *.cnf *.drat
	rm -f xor-eg xstrip xtree

