SH=/bin/sh
INTERP = python3
FORMAT = ./format_data.py

run:
	cd chess-kissat; $(SH) run.sh
	cd chess-scan; $(SH) run.sh
	cd pigeon-kissat; $(SH) run.sh
	cd pigeon-scan; $(SH) run.sh
	cd chew-kissat; $(SH) run.sh
	cd chew-bucket; $(SH) run.sh
	cd chew-gauss; $(SH) run.sh
	cd urquhart-bucket; $(SH) run.sh
	cd urquhart-gauss; $(SH) run.sh
	echo "EXPERIMENTAL RESULTS:"
	$(INTERP) $(FORMAT) | tee results-measured.txt

krun:
	cd chess-kissat; $(SH) run.sh
	cd pigeon-kissat; $(SH) run.sh
	cd chew-kissat; $(SH) run.sh
	echo "EXPERIMENTAL RESULTS:"
	$(INTERP) $(FORMAT) | tee results-measured.txt


clean:
	cd chess-kissat; make clean
	cd chess-scan; make clean
	cd pigeon-kissat; make clean
	cd pigeon-scan; make clean
	cd chew-kissat; make clean
	cd chew-bucket; make clean
	cd chew-gauss; make clean
	cd urquhart-bucket; make clean
	cd urquhart-gauss; make clean
	rm -f *~ *.pyc
