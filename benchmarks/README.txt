This directory contains the code to generate a set of challenging benchmarks for SAT solvers:

chew:
	Parity formulas described by Chew & Heule,
	"Sorting parity encodings by reusing variables", SAT 2020
	Parameterized by formula variables N and random seed
	BDDs can solve efficiently using bucket elimination

chess:
	The mutilated chessboard problem.
	Parameterized by board dimension N
	BDDs can solve efficiently using the generated schedules
	The .order files can be ignored.

pigeon-sinz:
	The pigeonhole problem, using Sinz's encoding of the at-most-one constraints
	Sinz, "Towards and optimal CNF encoding of Boolean cardinality constraints",
	Constraint Programming, 2005
	Parameterized by number of holes N (pigeons = N+1)
	BDDs can solve efficiently using the generated schedules.
	The .order files can be ignored.

urquhart-li:
	Parity formulas devised by Urquhart using a benchmark generator
	written by Li.
	Urquhart, "The complexity of propositional proofs",
	Bulletin of Symbolic Logic, 1995
	Parameterized by M, where graph contains 2*M*M nodes
	BDDs can solve efficiently using bucket elimination

Setting up:
	From this directory, run 'make generate'.  That will
	compile the needed code and generate a representative set
	of instances for each benchmark

Cleaning:
	From this directory, run 'make clean'.  That will
	eliminate all compiled code and remove the generated files


