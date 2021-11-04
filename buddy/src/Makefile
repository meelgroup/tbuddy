CC=gcc
CXX=g++
CFLAGS=-g -O2 -Wno-nullability-completeness -DCACHESTATS
INC = -I..
LDIR = ../lib
IDIR = ../include

FILES = bddio.o bddop.o bvec.o cache.o fdd.o imatrix.o kernel.o pairs.o prime.o reorder.o tree.o cppext.o

all: buddy.a
	cp -p buddy.a $(LDIR)
	cp -p bdd.h $(IDIR)

buddy.a: $(FILES)
	ar cr buddy.a $(FILES)

bddtest: bddtest.cxx buddy.a
	$(CXX) $(CFLAGS) -o bddtest bddtest.cxx buddy.a

.SUFFIXES: .c .cxx .o

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $<

.cxx.o:
	$(CXX) $(CFLAGS) $(INC) -c $<

clean:
	 rm -f bddtest *.a *.o *~


