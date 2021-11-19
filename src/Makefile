CC=gcc
CXX=g++
OPT=-O2
CFLAGS=-g $(OPT) -Wno-nullability-completeness
INC=-I../buddy/include
LDIR = ../buddy/lib
LIB = $(LDIR)/buddy.a
TLIB = $(LDIR)/tbuddy.a


bsat: clause.cpp clause.h eval.cpp bsat.cpp 
	$(CXX) $(CFLAGS) $(INC) -o bsat clause.cpp eval.cpp bsat.cpp $(LIB)

tbsat: clause.cpp clause.h teval.cpp bsat.cpp 
	$(CXX) $(CFLAGS) $(INC) -o tbsat clause.cpp teval.cpp bsat.cpp $(TLIB)


bsat-TBDD: clause.cpp clause.h eval-TBDD.cpp bsat.cpp 
	$(CXX) $(CFLAGS) $(INC) -o bsat-TBDD clause.cpp eval-TBDD.cpp bsat.cpp $(TLIB)


ctest: ctest.cpp clause.cpp clause.h
	$(CXX) $(CFLAGS) -o ctest ctest.cpp clause.cpp 

evalhack.o: evalhack.c evalhack.h
	$(CC) $(CFLAGS) $(INC) -c evalhack.c

.SUFFIXES: .c .cpp .o

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $<

.cpp.o:
	$(CXX) $(CFLAGS) $(INC) -c $<

clean:
	rm -f *.o *~
	rm -f bsat ctest tbsat bsat-TBDD

