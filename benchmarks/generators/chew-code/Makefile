CXX=g++
CFLAGS = -std=c++11 -g -O2 -Wall 

parity-cnf: Source.cpp
	$(CXX) $(CFLAGS) Source.cpp -DCNF_ONLY -o parity-cnf

all: parity parity-cnf

parity: Source.cpp
	$(CXX) $(CFLAGS) Source.cpp -o parity

clean:
	rm -f parity parity-cnf
	rm -rf *~ *.dSYM

