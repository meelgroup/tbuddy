CC=gcc
CXX=g++
TFLAGS =  -DENABLE_TBDD -DENABLE_BTRACE
#DFLAGS =  -DCACHESTATS -DINSTRUMENT
DFLAGS =  -DCACHESTATS
OPT = -O2
#OPT = -O0
CFLAGS=-g $(OPT) -fPIC -Wno-nullability-completeness $(DFLAGS)
CXXFLAGS=-std=c++11
INC = -I..
LDIR = ../lib
IDIR = ../include

# PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif


FILES = bddio.o bddop.o bvec.o cache.o fdd.o ilist.o imatrix.o kernel.o pairs.o prime.o reorder.o tree.o cppext.o

TFILES = tbdd.to prover.to \
	bddio.to bddop.to bvec.to cache.to fdd.to ilist.to imatrix.to kernel.to pairs.to prime.to reorder.to tree.to cppext.to \
	pseudoboolean.to

all: libbuddy.a libtbuddy.a libtbuddy.so
	rm -rf $(LDIR)
	mkdir $(LDIR)
	rm -rf $(IDIR)
	mkdir $(IDIR)
	cp -p libbuddy.a $(LDIR)
	cp -p libtbuddy.a $(LDIR)
	cp -p bdd.h $(IDIR)
	cp -p tbdd.h $(IDIR)
	cp -p ilist.h $(IDIR)
	cp -p prover.h $(IDIR)
	cp -p pseudoboolean.h $(IDIR)
	cp -p libtbuddy.so $(LDIR)

install: all
	install -m 644 libtbuddy.so $(PREFIX)/lib/
	install -m 644 bdd.h $(PREFIX)/include/
	install -m 644 ilist.h $(PREFIX)/include/
	install -m 644 prover.h $(PREFIX)/include/
	install -m 644 pseudoboolean.h $(PREFIX)/include/
	install -m 644 tbdd.h $(PREFIX)/include/


libbuddy.a: $(FILES)
	ar cr libbuddy.a $(FILES)

libtbuddy.so: $(TFILES)
	$(CXX) -shared -o libtbuddy.so $(TFILES)

libtbuddy.a: $(TFILES)
	ar cr libtbuddy.a $(TFILES)

bddtest: bddtest.cxx libbuddy.a
	$(CXX) $(CFLAGS) -o bddtest bddtest.cxx libbuddy.a

.SUFFIXES: .c .cxx .o .to

.c.o:
	$(CC) $(CFLAGS) $(INC) -c $<

.cxx.o:
	$(CXX) $(CFLAGS) $(INC) -c $<

.c.to:
	$(CC) $(CFLAGS) $(TFLAGS) $(INC) -c $< -o $@

.cxx.to:
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(TFLAGS) $(INC) -c $< -o $@

clean:
	 rm -f bddtest *.a *.o *.to *~


