
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
    print("Usage: %s [-h] [-v] [-p PATH] [-t TLIM] [-m SMIN] [-d SINCR] [-h SMAX] [-l LOG]")
    print("  -h         Print this message")
    print("  -v         Print generated results")
    print("  -p PATH    Directory containing benchmark")
    print("  -t TLIM    Time limit")
    print("  -m SMIN    Minimum size parameter")
    print("  -d SINCR   Size increment")
    print("  -h SMAX    Maximum size)
    print("  -l LOG     Log file")

makePath = "/usr/bin/make"

# Mapping from size to result
resultDict = {}

verbose = False
# Print info on stdout?
trace = True
# How much extra time to allow on final run
slack = 1.1
# What to look for in output stream
okrunword = "UNSAT"
okcheckword = "VERIFIED"

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
    # Allow a little extra so that timeout unambiguous
    wlog("Running size %d" % size)
    tlim = timelimit + 3
    alist = [makePath, "run", "SIZE=%d" % size, "TLIM=%d" % tlim]
    start = datetime.datetime.now()
    ok = False
    p = subprocess.Popen(alist, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    try:
        out, errs = p.communicate(timeout=timelimit)
        outs = out.decode('utf8')
        if verbose:
            print(outs)
        if okrunword in outs:
            ok = True
    except subprocess.TimeoutExpired:
        p.kill()
        wlog("Timed out size %d after %.3f" % (size, timelimit))
        return -1
    if p.returncode != 0:
        wlog("Benchmark exited with return code %d" % p.returncode, True)
        return -1
    if not ok:
        wlog("Benchmark did not generate '%s' for size %d" % (okrunword, size))
        return -1
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Ran size %d in time %.3f" % (size, secs))
    return secs
    
def docheck(path, size):
    alist = [makePath, "check", "SIZE=%d" % size]
    start = datetime.datetime.now()
    p = subprocess.Popen(alist, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    out, errs = p.communicate()
    outs = out.decode('utf8')
    if verbose:
        print(outs)
    ok = okcheckword in outs
    if p.returncode != 0:
        wlog("ERROR: Checker exited with return code %d" % p.returncode)
        return -1
    if not ok:
        wlog("Checker did not generate '%s' for size %d" % (okcheckword, size))
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
        wlog('ERROR: Checker exited with return code %d' % p.returncode)
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


# Choose next size
def nextsize(low, high, incrsize):
    l = low//incrsize
    h = high//incrsize
    m = (l+h)//2
    return m*incrsize

# maxsize = None requires scaling up to find upper bound
def probe(path, minsize, maxsize, incrsize, timelimit):
    start = datetime.datetime.now()
    psize = 0
    size = minsize
    t = 0.0
    # Scale up until exceed bound
    if maxsize is None:
        while True:
            t = dopass(path, size, timelimit)
            if t < 0:
                break
            psize = size
            size *= 2
        if psize == 0:
            wlog('Failed for minimum size %d' % minsize, True)
            sys.exit(1)
        lsize = psize
        hsize = size
    else:
        wlog('Making sure lower bound of %d is valid' % minsize)
        if dopass(path, minsize, timelimit) < 0:
            wlog('Failed for minimum size %d' % minsize, True)        
            sys.exit(1)
        lsize = minsize
        hsize = maxsize

    # Binary searching.
    # Invariant: lsize is ok.  hsize is too big
    while True:
        size = nextsize(lsize, hsize, incrsize)
        if size == lsize:
            wlog("Converged: lsize:%d, size:%d, hsize:%d" % (lsize, size, hsize))
            break
        t = dopass(path, size, timelimit)
        if t < 0:
            hsize = size
        else:
            lsize = size
    # Final run
    t = dorun(path, size, timelimit*slack)
    docheck(path, size)
    doclear(path, size)
    wlog("Completed size %d in time %.3f" % (size, t), True)
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Overall time for search: %.3f" % secs)
    
def run(name, arglist):
    path = '.'
    minsize = 1
    maxsize = None
    incrsize = 1
    timelimit = 1000.0
    global logfile, verbose
    optlist, args = getopt.getopt(arglist, "hvp:m:d:t:l:h:")
    for (opt, val) in optlist:
        if opt == '-h':
            usage(name)
            return
        elif opt == '-v':
            verbose = True
        elif opt == '-p':
            path = val
        elif opt == '-m':
            minsize = int(val)
        elif opt == '-h':
            maxsize = int(val)
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
    probe(path, minsize, maxsize, incrsize, timelimit)

if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])
