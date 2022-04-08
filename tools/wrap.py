#!/usr/bin/python

# Time the execution of a program.  Force termination if that program exceeds a time limit

import sys
import subprocess
import datetime
import os.path

def usage(name):
    print("Usage: %s TLIM PATH Args ..." % name)
    print("    TLIM:    Runtime limit (in seconds)")
    print("    PATH:    Path of executable program")
    print("    Args ... Arguments to pass to invoked program")

def runprog(timelimit, path, arglist):
    alist = [path] + arglist
    start = datetime.datetime.now()
    p = subprocess.Popen(alist)
    try:
        p.wait(timeout=timelimit)
    except subprocess.TimeoutExpired:
        p.kill()
        print("Execution of %s FAILED.  Timed out after %d seconds" % (path, timelimit))
        sys.exit(1)
    delta = datetime.datetime.now() - start
    secs = delta.seconds + 1e-6 * delta.microseconds
    print("Program %s completed with exit code %d" % (path, p.returncode))
    print("Wrapped time: %.3f seconds" % secs)
    return p.returncode
    
def run(name, arglist):
    if len(arglist) < 2:
        usage(name)
        return
    try:
        timelimit = float(arglist[0])
    except:
        print("Invalid time limit '%s'" % arglist[0])
        usage(name)
        return
    path = arglist[1]
    if not os.path.exists(path):
        print("Invalid path '%s'" % path)
        usage(name)
        return
    arglist = arglist[2:]
    code = runprog(timelimit, path, arglist)
    sys.exit(code)
          

name = sys.argv[0]
arglist = sys.argv[1:]
run(name, arglist)
    
    
