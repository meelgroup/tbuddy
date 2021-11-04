CXX=g++
CFLAGS = -g -O2 -Wall  -Wno-nullability-completeness

generate: main.cpp generate.cpp graph.cpp
	$(CXX) $(CFLAGS) -o generate main.cpp generate.cpp graph.cpp

clean:
	rm -f generate
	rm -rf *~ *.dSYM


