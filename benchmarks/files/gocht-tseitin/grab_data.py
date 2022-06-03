#!/usr/bin/python3

# Extract data from Gocht's paper results
# Have already used grep to get relevant lines
# Now want to know instance, SAT time, and verification time


import csv
import sys

def problemSize(rdict):
    name = rdict['instance.path']
    fields = name.split('_')
    sfield = fields[1]
    snum = sfield[1:]
    return int(snum)

def satTime(rdict):
    snum = rdict['solver.wallClockTime']
    return float(snum)

def checkTime(rdict):
    snum = rdict['drat_trim.wallClockTime']
    if snum == "NA":
        snum = rdict['veripb.wallClockTime']
    return float(snum)

def run(name, args):
    if len(args) == 0 or args[0] == '-h':
        print("Usage: %s CSVROOT")
        return
    root = args[0]
    iname = root + ".csv"
    try:
        ifile = open(iname, 'r')
    except:
        print("Couldn't open '%s'" % iname)
        return
    reader = csv.DictReader(ifile)

    sdict = {}
    cdict = {}


    for row in reader:
        n = problemSize(row)
        st = satTime(row)
        ct = checkTime(row)
        sdict[n] = st
        cdict[n] = ct

    sname = root + "-sat.csv"
    sfile = open(sname, 'w')
    cname = root + "-check.csv"
    cfile = open(cname, 'w')

    for n in sorted(sdict.keys()):
        t = sdict[n]
        sfile.write("%d,%.4f\n" % (n, t))

    for n in sorted(cdict.keys()):
        t = cdict[n]
        cfile.write("%d,%.4f\n" % (n, t))


if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])
