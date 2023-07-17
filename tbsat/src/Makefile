CC=gcc
CXX=g++
OPT=-O2
#OPT=-O0
CFLAGS=-g $(OPT) -Wno-nullability-completeness -std=c++11
INC=-I../../buddy/include
LDIR = ../../buddy/lib
LIB = $(LDIR)/libbuddy.a

all: tbsat sat_check

bsat: clause.cpp clause.h eval.cpp bsat.cpp 
	$(CXX) $(CFLAGS) $(INC) -o bsat clause.cpp eval.cpp bsat.cpp $(LIB)

.SUFFIXES: .c .cpp .o

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $<

.cpp.o:
	$(CXX) $(CFLAGS) $(INC) -c $<

clean:
	rm -f *.o *~
	rm -f bsat ctest tbsat

