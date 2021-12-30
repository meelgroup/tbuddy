#!/usr/bin/python

# Convert CNF file containing only xor declarations into
# schedule file that converts these into pseudo-Boolean equations
# triggers Gauss-Jordan elimination
# and loads any remaining clauses.
# The solver should then shift to bucket elimination

import getopt
import sys
import glob

import exutil


def usage(name):
    exutil.ewrite("Usage: %s [-v VLEVEL] [-h] [-c] [-i IN.cnf] [-o OUT.schedule] [-d DIR] [-m MAXCLAUSE]\n" % name, 0)
    exutil.ewrite("  -h       Print this message\n", 0)
    exutil.ewrite("  -v VERB  Set verbosity level (1-4)\n", 0)
    exutil.ewrite("  -c       Careful checking of CNF\n", 0)
    exutil.ewrite("  -i IFILE Single input file\n", 0)
    exutil.ewrite("  -i OFILE Single output file\n", 0)
    exutil.ewrite("  -p PATH  Process all CNF files with matching path prefix\n", 0)
    exutil.ewrite("  -m MAXC  Skip files with larger number of clauses\n", 0)


class Xor:
    # Input clauses
    clauses = []
    # Mapping from list of variables to the clauses containing exactly those variables
    varMap = {}
    msgPrefix = ""

    def __init__(self, clauses, iname):
        self.clauses = clauses
        self.varMap = {}
        self.msgPrefix = "" if iname is None else "File %s: " % iname
        for idx in range(1, len(clauses)+1):
            clause = self.getClause(idx)
            clause.sort(key = lambda lit : abs(lit))
            vars = tuple(sorted([abs(l) for l in clause]))
            if vars in self.varMap:
                self.varMap[vars].append(idx)
            else:
                self.varMap[vars] = [idx]
            
    def getClause(self, idx):
        if exutil.careful and (idx < 1 or idx > len(self.clauses)):
            raise self.msgPrefix + "Invalid clause index %d.  Allowed range 1 .. %d" % (idx, len(self.clauses))
        return self.clauses[idx-1]

    # Given set of clause IDs  over common set of variables,
    # Return partitioning into three sets: xors, xnors, other
    def classifyClauses(self, idlist):
        xorClauses = []
        xnorClauses = []
        otherClauses = []
        xorNvals = []
        xnorNvals = []
        if len(idlist) == 0:
            return (xorClauses, xnorClause, otherClauses)
        c0 = self.getClause(idlist[0])
        tcount = 2**(len(c0)-1)
        for id in idlist:
            clause = self.getClause(id)
            nlist = [1 if lit < 0 else 0 for lit in clause]
            nval = sum([nlist[i] * 2**i for i in range(len(nlist))])
            # Xor will have even number of negative literals
            if sum(nlist) % 2 == 0:
                xorClauses.append(id)
                xorNvals.append(nval)
            else:
                xnorClauses.append(id)
                xnorNvals.append(nval)
        # See if have true xors or true xnors
        nset = set(xorNvals)
        if len(nset) != tcount:
            otherClauses = otherClauses + xorClauses
            xorClauses = []
        nset = set(xnorNvals)
        if len(nset) != tcount:
            otherClauses = otherClauses + xnorClauses
            xnorClauses = []
        return (xorClauses, xnorClauses, otherClauses)
        
    def emitX(self, idlist, phase, outfile):
        slist = [str(id) for id in idlist]
        outfile.write("c %s\n" % " ".join(slist))
        if (len(idlist) > 1):
            outfile.write("a %d\n" % (len(idlist)-1))
        vars = [abs(lit) for lit in self.getClause(idlist[0])]
        stlist = ['1.%d' % v for v in vars]
        outfile.write("=2 %d %s\n" % (phase, " ".join(stlist)))

    def generate(self, oname):
        if oname is None:
            outfile = sys.stdout
        else:
            try:
                outfile = open(oname, 'w')
            except:
                exutil.ewrite("%sCouldn't open output file '%s'\n" % (self.msgPrefix, oname), 1)
                return False
        idlists = list(self.varMap.values())
        clauseCount = 0
        xcount = 0
        otherIdList = []
        for idlist in idlists:
            clauseCount += len(idlist)
            (xorClauses, xnorClauses, otherClauses) = self.classifyClauses(idlist)
            if len(xorClauses) > 0:
                self.emitX(xorClauses, 1, outfile)
                xcount += 1
            if len(xnorClauses) > 0:
                self.emitX(xnorClauses, 0, outfile)
                xcount += 1
            otherIdList += otherClauses
        if (xcount > 0):
            outfile.write("g\n")
        if len(otherIdList) > 0:
            slist = [str(id) for id in sorted(otherIdList)]
            outfile.write("c %s\n" % " ".join(slist))
        if oname is not None:
            outfile.close()
        exutil.ewrite("%s%d equations extracted.  %d other clauses.  %d total clauses\n" % (self.msgPrefix, xcount, len(otherIdList), clauseCount), 1)
        return True
        
def extract(iname, oname, maxclause):
    try:
        reader = exutil.CnfReader(iname, maxclause = maxclause, rejectClause = None)
        if len(reader.clauses) == 0:
            exutil.ewrite("File %s contains more than %d clauses\n" % (iname, maxclause), 2)
            return False
    except Exception as ex:
        exutil.ewrite("Couldn't read CNF file: %s" % str(ex), 1)
        return
    xor = Xor(reader.clauses, iname)
    return xor.generate(oname)


def replaceExtension(path, ext):
    fields = path.split('.')
    if len(fields) == 1:
        return path + '.' + ext
    else:
        fields[-1] = ext
    return ".".join(fields)

def run(name, args):
    iname = None
    oname = None
    path = None
    maxclause = None
    ok = True

    optlist, args = getopt.getopt(args, "hcv:i:o:p:m:")
    for (opt, val) in optlist:
        if opt == '-h':
            ok = False
        elif opt == '-v':
            exutil.verbLevel = int(val)
        elif opt == '-c':
            exutil.careful = True
        elif opt == '-i':
            iname = val
        elif opt == '-o':
            oname = val
            exutil.errfile = sys.stdout
        elif opt == '-p':
            path = val
            exutil.errfile = sys.stdout
        elif opt == '-m':
            maxclause = int(val)
    if not ok:
        usage(name)
        return
    if path is None:
        ecode = 0 if  extract(iname, oname, maxclause) else 1
        sys.exit(ecode)
    else:
        if iname is not None or oname is not None:
            exutil.ewrite("Cannot specify path + input or output name", 0)
            usage(name)
            sys.exit(0)
        scount = 0
        flist = sorted(glob.glob(path + '*.cnf'))
        for iname in flist:
            oname = replaceExtension(iname, 'schedule')
            if extract(iname, oname, maxclause):
                scount += 1
        exutil.ewrite("Extracted equations for %d/%d files\n" % (scount, len(flist)), 1)

if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])
        
            
            
