
# Use binary search to find maximum parameter for benchmark that runs within specified time

################################################################################
#
# Assumes that Makefile set up in directory supporting following operations
#
# make gen SIZE=XXX          Generate benchmark
# make run SIZE=XXX          Run solver
# make check SIZE=XXX        Check proof
# make clear SIZE=XXX        Delete proof file
#
################################################################################

import sys
import subprocess
import datetime
import os.path
import getopt


def usage(name):
    print("Usage: %s [-h] [-p PATH] [-t TLIM] [-m SMIN] [-d SINCR] [-l LOG]")
    print("  -p PATH    Directory containing benchmark")
    print("  -t TLIM    Time limit")
    print("  -m SMIN    Minimum size parameter")
    print("  -d SINCR   Size increment")
    print("  -l LOG     Log file")

makePath = "/usr/bin/make"

# Mapping from size to result
resultDict = {}

trace = True
# How much extra time to allow on final run
slack = 1.1

logfile = None


def wlog(msg, strong = False):
    if logfile is None:
        print("PROBE:" + msg)
        return
    logfile.write("PROBE:" + msg + '\n')
    if trace or strong:
        print("PROBE:" + msg)

def dogenerate(path, size):
    alist = [makePath, "gen", "SIZE=%d" % size]
    start = datetime.datetime.now()
    p = subprocess.Popen(alist)
    p.wait()
    if p.returncode != 0:
        wlog("ERROR: Generator exited with return code %d" % p.returncode, True)
        return -1
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Generated size %d in time %.3f" % (size, secs))
    return secs

def dorun(path, size, timelimit):
    alist = [makePath, "run", "SIZE=%d" % size]
    start = datetime.datetime.now()
    p = subprocess.Popen(alist)
    try:
        p.wait(timeout=timelimit)
    except subprocess.TimeoutExpired:
        p.kill()
        wlog("Timed out size %d after %.3f" % (size, timelimit))
        return -1
    if p.returncode != 0:
        wlog("Benchmark exited with return code %d" % p.returncode, True)
        return -1
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Ran size %d in time %.3f" % (size, secs))
    return secs

def docheck(path, size):
    alist = [makePath, "check", "SIZE=%d" % size]
    start = datetime.datetime.now()
    p = subprocess.Popen(alist)
    p.wait()
    if p.returncode != 0:
        wlog("ERROR: Checker exited with return code %d" % p.returncode)
        return -1
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Checked size %d in time %.3f" % (size, secs))
    return secs

def doclear(path, size):
    if size == 0:
        return 0
    alist = [makePath, "clear", "SIZE=%d" % size]
    start = datetime.datetime.now()
    p = subprocess.Popen(alist)
    p.wait()
    if p.returncode != 0:
        wlog("ERROR: Checker exited with return code %d" % p.returncode)
        return -1
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    return secs

def dopass(path, size, timelimit):
    if size in resultDict:
        return resultDict[size]
    if dogenerate(path, size) < 0:
        wlog("Testing failed.  Exiting", True)
        sys.exit(1)
    t = dorun(path, size, timelimit)
    doclear(path, size)
    resultDict[size] = t
    return t


def probe(path, minsize, incrsize, timelimit):
    start = datetime.datetime.now()
    psize = 0
    size = minsize
    t = 0.0
    # Scale up until exceed bound
    while True:
        t = dopass(path, size, timelimit)
        if t < 0:
            break
        psize = size
        size *= 2
    if psize == 0:
        wlog("Failed for minimum size %d" % minsize, True)
        sys.exit(1)


    # Binary searching.
    # Invariant: lsize is ok.  hsize is too big
    lsize = psize
    hsize = size
    while True:
        l = lsize//incrsize
        h = hsize//incrsize
        m = (l+h)//2
        size = m*incrsize
        if size == lsize:
            wlog("Converged: lsize:%d, size:%d, hsize:%d" % (lsize, size, hsize))
            break
        t = dopass(path, size, timelimit)
        if t < 0:
            hsize = size
        else:
            lsize = size
    # Final run
    dorun(path, size, timelimit*slack)
    docheck(path, size)
    doclear(path, size)
    wlog("Completed size %d in time %.3f" % (size, t), True)
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Overall time for search: %.3f" % secs)
    
def run(name, arglist):
    path = '.'
    minsize = 1
    incrsize = 1
    timelimit = 1000.0
    global logfile
    optlist, args = getopt.getopt(arglist, "hp:m:d:t:l:")
    for (opt, val) in optlist:
        if opt == '-h':
            usage(name)
            return
        elif opt == '-p':
            path = val
        elif opt == '-m':
            minsize = int(val)
        elif opt == '-d':
            incrsize = int(val)
        elif opt == '-t':
            timelimit = float(val)
        elif opt == '-l':
            try:
                logfile = open(val, 'w')
            except:
                print("Couldn't open log file '%s'" % val)
                return
        else:
            print("Unknown option '%s'" % opt)
            usage(name)
            return
    probe(path, minsize, incrsize, timelimit)

if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])
