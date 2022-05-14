#!/usr/bin/python3

#####################################################################################
# Copyright (c) 2022 Randal E. Bryant, Carnegie Mellon University
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
# Step through FRAT file and try to match added clauses with
# equivalent input clauses.  As a side effect, also strip any comments
# ('c' command) from the file.

# Usage: ./frat-matcher.py < infile.frat > outfile.frat

# Map from string representation of clause to its ID
input_clauses = {}

def err_msg(msg):
    sys.stderr.write("ERROR: %s.  Exiting\n" % msg)
    sys.exit(1)

# Turn clause into canonical string
def gen_key(clause):
    ls = sorted(clause, key = lambda(s): abs(int(s)))
    slist = [str(v) for v in ls]
    return "+".join(slist)

def trim(s):
    while len(s) > 0 and s[-1] == '\n':
        s = s[:-1]
    return s

line_count = 0
no_hint_count = 0
new_hint_count = 0
clause_count = 0

for line in sys.stdin:
    line = trim(line)
    line_count += 1
    fields = line.split()
    if fields[0] == 'c':
        continue
    if len(fields) < 2:
        err_msg("Line #%d (%s) not valid" % (line_count, line))
    cmd = fields[0]
    id = int(fields[1])
    if (cmd == 'o'):
        clause_count += 1
        clause = fields[2:-1]
        input_clauses[gen_key(clause)] = id
    elif (cmd == 'a'):
        clause_count += 1
        clause = []
        hints = []
        foundl = False
        zcount = 0
        for field in fields[2:-1]:
            if field == 'l':
                if zcount != 1:
                    err_message("Line %d (%s).  Found 'l' before 0" % (line_count, line))
                foundl = True
            elif int(field) == 0:
                zcount += 1
            else:
                lit = int(field)
                if foundl:
                    hints.append(lit)
                else:
                    clause.append(lit)
        if len(hints) == 0:
            key = gen_key(clause)
            if key in input_clauses:
                hints = [input_clauses[key]]                
                new_hint_count += 1
                sfields = fields[0:2] + [str(lit) for lit in clause] + ['0'] + ['l'] + [str(hint) for hint in hints] + ['0']
                line = " ".join(fields)
            else:
                no_hint_count += 1
    sys.stdout.write(line + '\n')

sys.stderr.write("Read %d lines\n" % line_count)
sys.stderr.write("Found %d clauses\n" % clause_count)
sys.stderr.write("Added %d hints\n" % new_hint_count)
sys.stderr.write("%d clauses without hints\n" % no_hint_count)
