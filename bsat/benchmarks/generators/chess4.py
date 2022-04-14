#!/usr/bin/python3

#####################################################################################
# Copyright (c) 2022 Marijn Heule, Randal E. Bryant, Carnegie Mellon University
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
import  getopt
import random

import writer


# Generate files for chessboard tiling problem with 1x4 tiles,
# as communicated by Bernardo Subercaseaux
def usage(name):
    print("Usage: %s [-h] [-C] [-v] [-r ROOT] -n N [-w n|h|v|b]" % name) 
    print("  -h       Print this message")
    print("  -v       Run in verbose mode")
    print("  -C       Generate clauses only.  No order or schedule")
    print("  -r ROOT  Specify root name for files.  Will generate ROOT.cnf, and possibly ROOT.order, ROOT.schedule")
    print("  -w WRAP  Wrap board horizontally (h), vertically (v), both (b) or neither (n)")
    print("  -n N     Specify size of board")


def exactlyOne(vars):
    n = len(vars)
    if n == 0:
        return None # Can't do it
    # At least one
    clauses = [vars]
    # at most one via pairwise constraints
    for i in range(n):
        for j in range(i):
            clause = [-vars[i], -vars[j]]
            clauses.append(clause)
    return clauses


# Numbering Scheme
# Rows and columns numbered from 1 to n
# Left to right for rows, Top to bottom for columns

# Squares in row-major order: id(r,c) = (r-1) * n + c
# Indices range from 1 to n*n

# Each domino position is identified by the location of its upper left square
# plus whether it is oriented horizontally (H) or vertically (V)
# Horizontal dominos have 1 <= r <= n,   1 <= c <= n-3 (or 1 <= c <= n if wrap horizontally)
# Vertical dominos have   1 <= r <= n-3 (or 1 <= c <= n if wrap vertially), 1 <= c <= n 

class Domino:
    row = 1
    col = 1
    n = 1
    horizontal = True
    var = 1
    maxRow = 1
    maxCol = 1
    
    def __init__(self, row, col, n, isHorizontal, var):
        self.row = row
        self.col = col
        self.n = n
        self.horizontal = isHorizontal
        self.var = var
        self.maxRow = row if isHorizontal else row+3
        self.maxCol = col+3 if isHorizontal else col

    def addToDict(self, dominoDict):
        deltaRow = 0 if self.horizontal else 1
        deltaCol = 1 if self.horizontal else 0
        ls = []
        touched = []
        for i in range(4):
            r = (self.row-1 + i*deltaRow) % self.n + 1
            c = (self.col-1 + i*deltaCol) % self.n + 1
            if (r,c) in dominoDict:
                dominoDict[(r,c)].append(self)
            else:
                dominoDict[(r,c)] = [self]
            touched.append((r,c))
#        print("Domino #%d %s touches squares %s" % (self.var, self.name(), str(touched)))

    def name(self):
        return "%s(%d,%d)" % ("H" if self.horizontal else "V", self.row, self.col)
        
class Board:
    n = 8
    # List of all dominos
    dominos = []
    # Dictionary mapping (r,c) to list of all dominos overlapping square
    dominoDict = {}
    # List of dominos having given (decremented) column as rightmost position
    columnDominos = []
    # List of all dominos having given (decremented) row as starting position
    rowDominos = []
    # List of all clause numbers that are associated with each (decremented) column
    columnClauses = []

    docStrings = []

    def __init__(self, n, wrapHorizontal, wrapVertical):
        self.n = n
        self.documentStrings = ["Tiling %d x %d chessboard with 1x4 and 4x1 dominos" % (n,n),
                                "Wrap horizontal: %s.  Wrap vertical: %s" % 
                                ("yes" if wrapHorizontal else "no", "yes" if wrapVertical else "no")]
        self.dominos = []
        self.dominoDict = {}
        self.columnDominos = [[] for i in range(n)]
        self.rowDominos = [[] for i in range(n)]
        self.columnClauses = [[] for i in range(n)]
        nextVar = 1
        maxhc = n if wrapHorizontal else n-3
        maxvr = n if wrapVertical else n-3
        for r in range(1,n+1):
            for c in range(1,n+1):
                # Horizontal domino
                if c <= maxhc:
                    d = Domino(r, c, n, True, nextVar)
                    nextVar += 1
                    self.dominos.append(d)
                    d.addToDict(self.dominoDict)
                    self.columnDominos[d.maxCol-1].append(d)
                    self.rowDominos[r-1].append(d)
                if r <= maxvr:
                    d = Domino(r, c, n, False, nextVar)
                    nextVar += 1
                    self.dominos.append(d)
                    d.addToDict(self.dominoDict)
                    self.columnDominos[d.maxCol-1].append(d)        
                    self.rowDominos[r-1].append(d)
       
    def genCnf(self, froot, verbose = False):
        cnf = writer.CnfWriter(len(self.dominos), froot, verbose)
        for line in self.documentStrings:
            cnf.doComment(line)
        nextClause = 1
        for r in range(1,self.n+1):
            for c in range(1, self.n+1):
                if verbose:
                    cnf.doComment("Constraints for dominos covering square(%d,%d)" % (r, c))
                vars = [d.var for d in self.dominoDict[(r,c)]]
                clauses = exactlyOne(vars)
                for cls in clauses:
                    cnf.doClause(cls)
                    self.columnClauses[c-1].append(nextClause)
                    nextClause += 1
        cnf.finish()

    def genSchedule(self, froot, verbose = False):
        sched = writer.ScheduleWriter(len(self.dominos), froot, verbose)
        if verbose:
            for line in self.documentStrings:
                sched.doComment(line)
        sched.newTree()
        for c in range(1, self.n+1):
            if verbose:
                sched.doComment("Process clauses for column %d" % c)
            clauses = self.columnClauses[c-1]
            sched.getClauses(clauses)
            sched.doAnd(len(clauses))
            vlist = [d.var for d in self.columnDominos[c-1]]
            sched.doQuantify(vlist)
            sched.doInformation("After quantifying variables for column %d" % c)
        sched.finish()
            
    def genOrder(self, froot):
        order = writer.OrderWriter(len(self.dominos), froot, False, "order")
        for r in range(1, self.n+1):
            vlist = [d.var for d in self.rowDominos[r-1]]
            order.doOrder(vlist)
        order.finish()
    
                           
def run(name, args):
    verbose = False
    n = 0
    rootName = None
    wrapHorizontal = False
    wrapVertical = False    
    clauseOnly = False
    optlist, args = getopt.getopt(args, "hvCr:n:w:")
    for (opt, val) in optlist:
        if opt == '-h':
            usage(name)
            return
        elif opt == '-v':
            verbose = True
        elif opt == '-C':
            clauseOnly = True
        elif opt == '-r':
            rootName = val
        elif opt == '-n':
            n = int(val)
        elif opt == '-w':
            if len(val) != 1 or val not in "nhvb":
                print("Invalid wrap specification '%s'" % val)
                usage(name)
                return
            if val in "hb":
                wrapHorizontal = True
            if val in "vb":
                wrapVertical = True

    if n == 0:
        print("Must have value for n")
        usage(name)
        return
    if rootName is None:
        print("Must have root name")
        usage(name)
        return
    b = Board(n, wrapHorizontal, wrapVertical)

    b.genCnf(rootName, verbose)
    if not clauseOnly:
        b.genSchedule(rootName, verbose)
        b.genOrder(rootName)

if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])

