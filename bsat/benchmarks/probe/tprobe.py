#!/usr/bin/python3

# Use binary search to find maximum parameter for benchmark that runs within specified time
# Specialized to benchmarks for tbuddy and its competitors

################################################################################
#
# Assumes that Makefile set up in directory supporting following operations
#
# make gen SIZE=XXX          Generate benchmark
# make check SIZE=XXX        Check proof
# make clear SIZE=XXX        Delete proof file
#
# Running solver is done directly
#
################################################################################

import os
import sys
import subprocess
import datetime
import os.path
import getopt


def usage(name):
    print("Usage: %s [-h] [-v VLEVEL] [-b BENCH]  [-t TLIM] [-m SMIN] [-d SINCR] [-u SMAX] [-s SEED] [-l LOG]")
    print("  -h         Print this message")
    print("  -v         Set verbosity level")
    print("  -b BENCH   Set benchmark.  Names of form PPPP-MMMM for problem PPPP and method MMMM")
    print("  -t TLIM    Time limit")
    print("  -m SMIN    Minimum size parameter")
    print("  -d SINCR   Size increment")
    print("  -d SEED    Benchmark seed")
    print("  -u SMAX    Maximum size")
    print("  -l LOG     Log file")

makePath = "/usr/bin/make"
mvPath = "/bin/mv"

# Mapping from size to result
resultDict = {}

verbose = False
# Print info on stdout?
trace = True
# What to look for in output stream
okrunword = "UNSAT"
okcheckword = "VERIFIED"

logfile = None

# Benchmark parameters
bench = None
problem = None
method = None
vlevel = 1
timelimit = 3600
seed = None

# Capabilities required to Assemble commandline argument for solver
reporoot = "/Users/bryant/repos/"

progpath = {
    "bucket": reporoot + "tbuddy/bsat/src/tbsat",
    "gauss":  reporoot + "tbuddy/bsat/src/tbsat",
    "scan":   reporoot + "tbuddy/bsat/src/tbsat",
    "pgbdd":  reporoot + "pgbdd/prototype/solver.py",
    "pgpbs":  reporoot + "pgbdd/prototype/solver.py",
    "check":  reporoot + "drat-trim/lrat-check",
    "chess":  reporoot + "tbuddy/bsat/benchmarks/generators/chess.py",
    "chew":   reporoot + "tbuddy/bsat/benchmarks/generators/chew-code/parity-cnf",
    "pigeon": reporoot + "tbuddy/bsat/benchmarks/generators/pigeon-sinz.py",
    "urquhart": reporoot + "tbuddy/bsat/benchmarks/generators/urquhart-li-code/generate"
}

extractorpath = {
    "gauss": reporoot + "tbuddy/tools/xor_extractor.py",
    "pgpbs": reporoot + "pgbdd/extractor/xor_extractor.py",
}


# Does program use schedule or bucket elimination?
def useschedule():
    if problem in ["chess", "pigeon"]:
        return True
    if method in ["gauss", "pgpbs"]:
        return True
    return False
    
def genfroot(size):
    froot = bench
    if seed is not None:
        froot += "-" + str(seed)
    froot += "-" + str(size)
    return froot

def genargs(size, final):
    froot = genfroot(size)
    args = [progpath[method], 
            "-v", str(vlevel),
            "-i", froot + ".cnf"]
    if final:
        args += ["-o", froot + ".lrat"]
    else:
        args += ["-o", froot + ".lrat"]
#        args += ["-o", "/dev/null"]
        args += ["-t", str(timelimit)]
    if useschedule():
        args += ["-s", froot + ".schedule"]
    else:
        args += ["-b"]
    return args

def wlog(msg, strong = False):
    if logfile is None:
        print("PROBE:" + msg)
        return
    logfile.write("PROBE:" + msg + '\n')
    if trace or strong:
        print("PROBE:" + msg)


# Run specified command, expressed as list with program followed by arguments
# Log results to data file, either with write or with append.
# Optionally scan for expected string
# Return True if execution OK (and string found)
def runprog(alist, froot, size, append=True, sstring = None):
    logfile = None if froot is None else froot + ".data"
    wlog("Running '%s'" % " ".join(alist))
    try:
        mode = 'a' if append else 'w'
        dout = open(logfile, mode)
    except Exception as ex:
        wlog("Failed to open output data file '%s' (%s)" % (logfile, str(ex)))
        return False
    p = subprocess.Popen(alist, stdout=dout, stderr=subprocess.STDOUT)
    p.wait()
    dout.close()
    if p.returncode != 0:
        wlog("ERROR: Program exited with return code %d" % p.returncode, True)
        return False
    if sstring is not None:
        try:
            din = open(logfile, 'r')
        except Exception as ex:
            wlog("Failed to open data file '%s' (%s) for reading" % (logfile, str(ex)))
            return False
        found = False
        for line in din:
            if sstring in line:
                found = True
                break
        din.close()
        if not found:
            wlog("String '%s' not found in data file %s" % (sstring, logfile))
            return False
    return True

def dogenerate(size):
    froot = genfroot(size)
    program = progpath[problem]
    alist = [program]
    if problem in ['chess', 'pigeon']:
        alist += ['-n', str(size), '-r', froot]
    elif problem == 'chew':
        alist += [str(size)]
        if seed is not None:
            alist += [str(seed)]
    elif problem == 'urquhart':
        alist += ["-m%s" % size, "-p50", "-f%s.cnf" % froot]
    start = datetime.datetime.now()
    ok = runprog(alist, froot, size, False)
    if not ok:
        return -1
    if problem == 'chew':
        ncnf = "%s.cnf" % froot
        os.rename("formula.cnf", ncnf)
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Generated size %d in time %.3f" % (size, secs))
    if method in extractorpath:
        program = extractorpath[method]
        alist = [program, "-i", "%s.cnf" % froot, "-o", "%s.schedule" % froot]
        ok = runprog(alist, froot, size)
        if not ok:
            return -1
    return secs

def dorun(size, final = False):
    froot = genfroot(size)
    # Allow a little extra so that timeout unambiguous
    alist = genargs(size, final)
    start = datetime.datetime.now()
    ok = runprog(alist, froot, size, sstring=okrunword)
    if not ok:
        return -1
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Ran size %d in time %.3f" % (size, secs))
    return secs
    
def docheck(size):
    froot = genfroot(size)
    program = progpath["check"]
    alist = [program, "%s.cnf" % froot, "%s.lrat" % froot]
    start = datetime.datetime.now()
    ok = runprog(alist, froot, size, sstring=okcheckword)
    if not ok:
        return -1
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Checked size %d in time %.3f" % (size, secs))
    return secs
    
def doclear(size):
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
    
def dopass(size):
    if size in resultDict:
        return resultDict[size]
    if dogenerate(size) < 0:
        wlog("Testing failed.  Exiting", True)
        sys.exit(1)
    t = dorun(size)
    resultDict[size] = t
    return t


# Choose next size
def nextsize(low, high, incrsize):
    l = low//incrsize
    h = high//incrsize
    m = (l+h)//2
    return m*incrsize

def probe(minsize, maxsize, incrsize):
    start = datetime.datetime.now()
    psize = 0
    size = minsize
    t = 0.0
    # Scale up until exceed bound
    if maxsize is None:
        while True:
            t = dopass(size)
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
        if dopass(minsize) < 0:
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
        t = dopass(size)
        if t < 0:
            hsize = size
        else:
            lsize = size
    # Final run
    t = dorun(size, final=True)
    docheck(size)
    doclear(size)
    wlog("Completed size %d in time %.3f" % (size, t), True)
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    wlog("Overall time for search: %.3f" % secs)
    
def run(name, arglist):
    global timelimit, bench, problem, method, vlevel, seed
    minsize = 1
    maxsize = None
    incrsize = 1
    timelimit = 1000.0
    global logfile, verbose
    optlist, args = getopt.getopt(arglist, "hv:m:b:d:t:l:u:s:")
    for (opt, val) in optlist:
        if opt == '-h':
            usage(name)
            return
        elif opt == '-v':
            vlevel = int(val)
        elif opt == '-m':
            minsize = int(val)
        elif opt == '-b':
            bench = val
        elif opt == '-u':
            maxsize = int(val)
        elif opt == '-d':
            incrsize = int(val)
        elif opt == '-t':
            timelimit = int(val)
        elif opt == '-s':
            seed = int(val)
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
    if bench is None:
        wlog("ERROR.  Need benchmark of form PPPP-MMMM")
        sys.exit(1)
    fields = bench.split("-")
    if len(fields) != 2:
        wlog("Invalid benchmark.  Must be of form PPPP-MMMM")
        sys.exit(1)
    problem, method = fields

    probe(minsize, maxsize, incrsize)

if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])
