INTERP=python3
TBUDDY = ~/repos/tbuddy
GRAB = $(TBUDDY)/tools/grab_data.py
PAIR = $(TBUDDY)/tools/grab_pair_data.py

data:
	$(INTERP) $(PAIR) d 1 "Elapsed" 1 "Total clauses" */*.ldata > all-tbsat-linear-pair.csv
	$(INTERP) $(PAIR) d 1 "Elapsed" 1 "Total clauses" */*.bdata > all-tbsat-bucket-pair.csv
	$(INTERP) $(PAIR) d 1 "Elapsed" 1 "Total clauses" */*.rdata > all-tbsat-rbucket-pair.csv
	$(INTERP) $(PAIR) d 1 "Elapsed" 1 "Total clauses" */*.sdata > all-tbsat-scan-pair.csv
	$(INTERP) $(PAIR) d 1 "Elapsed" 1 "Total clauses" */*.nqdata > all-tbsat-noquant-pair.csv
	$(INTERP) $(PAIR) d -1 "process-time" 1 "proof_added" */*.kdata | grep -v "0.0," > all-kissat-pair.csv

#	$(INTERP) $(PAIR) d 1 "verification time" 1 "Elapsed" */*.ldata  | grep -v "0.0,0" > all-tbsat-linear-check-pair.csv
#	$(INTERP) $(PAIR) d 1 "verification time" 1 "Elapsed" */*.bdata  | grep -v "0.0,0" > all-tbsat-bucket-check-pair.csv
#	$(INTERP) $(PAIR) d 1 "verification time" 1 "Elapsed" */*.rdata  | grep -v "0.0,0" > all-tbsat-rbucket-check-pair.csv
#	$(INTERP) $(PAIR) d 1 "verification time" 1 "Elapsed" */*.sdata  | grep -v "0.0,0" > all-tbsat-scan-check-pair.csv
#	$(INTERP) $(PAIR) d 1 "verification time" 1 "Elapsed" */*.nqdata  | grep -v "0.0,0" > all-tbsat-noquant-check-pair.csv
#	$(INTERP) $(PAIR) d 1 "verification time" -1 "process-time" */*.kdata | grep -v "0.0,0" | grep -v "1000000000000" > all-kissat-check-pair.csv

	$(INTERP) $(PAIR) d 1 "verification time" 1 "Total clauses" */*.ldata > all-tbsat-linear-check-pair.csv
	$(INTERP) $(PAIR) d 1 "verification time" 1 "Total clauses" */*.bdata > all-tbsat-bucket-check-pair.csv
	$(INTERP) $(PAIR) d 1 "verification time" 1 "Total clauses" */*.rdata > all-tbsat-rbucket-check-pair.csv
	$(INTERP) $(PAIR) d 1 "verification time" 1 "Total clauses" */*.sdata > all-tbsat-scan-check-pair.csv
	$(INTERP) $(PAIR) d 1 "verification time" 1 "Total clauses" */*.nqdata > all-tbsat-noquant-check-pair.csv
	$(INTERP) $(PAIR) d 1 "verification time" 1 "proof_added" */*.kdata | grep -v "0.0," > all-kissat-check-pair.csv


#	$(INTERP) $(GRAB) "Total clauses" */*.ldata > all-tbsat-linear-clauses.csv
#	$(INTERP) $(GRAB) "Total clauses" */*.bdata > all-tbsat-bucket-clauses.csv
#	$(INTERP) $(GRAB) "Total clauses" */*.rdata > all-tbsat-rbucket-clauses.csv
#	$(INTERP) $(GRAB) "Total clauses" */*.sdata > all-tbsat-scan-clauses.csv
#	$(INTERP) $(GRAB) "Total clauses" */*.nqdata > all-tbsat-noquant-clauses.csv
#	$(INTERP) $(GRAB) "proof_added" */*.kdata > all-kissat-clauses.csv
#	$(INTERP) $(GRAB) "Elapsed" */*.ldata > all-tbsat-linear-seconds.csv
#	$(INTERP) $(GRAB) "Elapsed" */*.bdata > all-tbsat-bucket-seconds.csv
#	$(INTERP) $(GRAB) "Elapsed" */*.rdata > all-tbsat-rbucket-seconds.csv
#	$(INTERP) $(GRAB) "Elapsed" */*.sdata > all-tbsat-scan-seconds.csv
#	$(INTERP) $(GRAB) "Elapsed" */*.nqdata > all-tbsat-noquant-seconds.csv
#	$(INTERP) $(GRAB) -1 "process-time" */*.kdata > all-kissat-seconds.csv
#	$(INTERP) $(GRAB) "verification time" */*.ldata > all-tbsat-linear-check-seconds.csv
#	$(INTERP) $(GRAB) "verification time" */*.bdata > all-tbsat-bucket-check-seconds.csv
#	$(INTERP) $(GRAB) "verification time" */*.rdata > all-tbsat-rbucket-check-seconds.csv
#	$(INTERP) $(GRAB) "verification time" */*.sdata > all-tbsat-scan-check-seconds.csv
#	$(INTERP) $(GRAB) "verification time" */*.nqdata > all-tbsat-noquant-check-seconds.csv
#	$(INTERP) $(GRAB) "verification time" */*.kdata > all-kissat-check-seconds.csv


clean:
	rm -f *~





