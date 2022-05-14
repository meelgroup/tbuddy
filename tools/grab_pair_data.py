#!/usr/bin/python3

#####################################################################################
# Copyright (c) 2021 Marijn Heule, Randal E. Bryant, Carnegie Mellon University
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
# associated documentation files (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge, publish, distribute,
# sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
# NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
# OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
########################################################################################

import sys
import re

# Generate csv of number specified on target line
# Extracts problem size from file name.

def usage(name):
    print("Usage: %s N1 TPHRASE1 N2 TPHRASE2 file ...")
    print("   Nx:       Look for Nth numeric field in line (count from 1)")
    print("   TPHRASEx: Trigger phrase identifying line")


# Will grep for lines containing each trigger phrase.
# If it contains a space, be sure to quote it.


triggerField1 = 1
triggerPhrase1 = "Total Clauses"

triggerField2 = 1
triggerPhrase2 = "verification time"

def trim(s):
    while len(s) > 0 and s[-1] == '\n':
        s = s[:-1]
    return s

dashOrDot = re.compile('[.-]')
def ddSplit(s):
    return dashOrDot.split(s)

colonOrSpace = re.compile('[\s:]+')
def lineSplit(s):
    return colonOrSpace.split(s)

def nthNumber(fields, n = 1):
    count = 0
    for field in fields:
        try:
            val = int(field)
            count += 1
            if count == n:
                return val
        except:
            try:
                val = float(field)
                count += 1
                if count == n:
                    return val
            except:
                continue
    return -1


def applyFilter(line, pos, trigger):
    line = trim(line)
    val = None
    if trigger in line:
        fields = lineSplit(line)
        val = nthNumber(fields, pos)
    return val
    

# Extract data from log.  Turn into something usable for other tools
def extract(fname):
    try:
        f = open(fname, 'r')
    except:
        print("Couldn't open file '%s'" % fname)
        return None
    val1 = None
    val2 = None
    for line in f:
        if val1 is None:
            val1 = applyFilter(line, triggerField1, triggerPhrase1)
        if val2 is None:
            val2 = applyFilter(line, triggerField2, triggerPhrase2)
    f.close()
    if val1 is None or val2 is None:
        return None
    return (val1, val2)

def run(name, args):
    if len(args) < 2 or args[0] == '-h':
        usage(name)
        return
    global triggerPhrase1, triggerPhrase2
    global triggerField1, triggerField2
    vals = {}
    try:
        triggerField1 = int(args[0])
        triggerPhrase1 = args[1]
        triggerField2 = int(args[2])
        triggerPhrase2 = args[3]
        names = args[4:]
    except:
        usage(name)
        return
    for fname in names:
        pair = extract(fname)
        if pair is not None:
            if pair[0] in vals:
                vals[pair[0]].append(pair)
            else:
                vals[pair[0]] = [pair]
    for k in sorted(vals.keys()):
        for p in vals[k]:
            print("%s,%s" % p)

if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])

            
        
                
