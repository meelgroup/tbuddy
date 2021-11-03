CC=gcc
CXX=g++
CFLAGS=-g -Wno-nullability-completeness
INC=-I../buddy/include
LDIR = ../buddy/lib
LIB = $(LDIR)/buddy.a


bsat: clause.cpp clause.h eval.cpp bsat.cpp
	$(CXX) $(CFLAGS) $(INC) -o bsat clause.cpp eval.cpp bsat.cpp $(LIB)

ctest: ctest.cpp clause.cpp clause.h
	$(CXX) $(CFLAGS) -o ctest ctest.cpp clause.cpp 

.SUFFIXES: .c .cpp .o

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $<

.cpp.o:
	$(CXX) $(CFLAGS) $(INC) -c $<

clean:
	rm -f *.o *~
	rm -f ctest

