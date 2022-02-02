#!/usr/bin/python

# Generate CNF encoding of Crawford's Minimal Disagreement Parity benchmark
# http://www.cs.cornell.edu/selman/docs/crawford-parity.pdf
# Use different encoding than presented in paper:
# - Incorporate constants directly into encoding
# - Use unary counter for at-most-k constraint

import random
import sys
import getopt

import writer


def usage(name):
    print("Usage: %s [-h] [-v] [-f] [-O] [-a] [-n N] [-m M] [-k K] [-t T] [-s SEED]" % name)
    print("  -h       Print this message")
    print("  -v       Put comments in file")
    print("  -f       Use fixed solution and corruption bits")
    print("  -O       Optimize (Plaisted-Greenbaum encoding)")
    print("  -a       Anonymize.  Don't include solution information in file")
    print("  -n N     Number of variables")
    print("  -m M     Number of samples")
    print("  -k K     Number of corrupted samples")
    print("  -t T     Number of mismatches tolerated")
    print("  -s SEED  Set random seed")


cwriter = None

# Parameters
verbose = False
optimize = False
seed = 123456
numVariables = 8
numSamples = 16
numCorrupt = 4
numTolerated = 4

# Bit vectors
solutionBits = []
corruptionBits = []

# numSamples random bit sequences, each of length numVariables
sampleBitSet = []
# The computed parity for each sample
parityBits = []
# The target phase for each sample
phaseBits = []

# Problem variables
solutionVariables = []
corruptionVariables = []


def randomBits(length):
    return [random.randint(0, 1) for i in range(length)]

def vectorParity(vec):
    parity = 0
    for bit in vec:
        parity ^= bit
    return parity

def vectorString(vec):
    slist = [str(v) for v in vec]
    return " ".join(slist)


def bitParity(w):
    value = 0
    while w != 0:
        value ^=  w & 0x1
        w = w >> 1
    return value

# Add list of clauses for encoding parity
def genParity(vars, phase):
    n = len(vars)
    for i in range(1 << n):
        # Count number of 0's in i
        zcount = bitParity(~i & ((1 << n) - 1))
        if zcount == phase:
            continue
        lits = []
        for j in range(n):
            weight = 1 if ((i>>j)&0x1) == 1 else -1
            lits.append(vars[j] * weight)
        cwriter.doClause(lits)

def genParityChain(vars, phase):
    if len(vars) <= 4:
        genParity(vars, phase)
    else:
        var = cwriter.newVariable()
        left = vars[0:2] + [var]
        right = vars[2:] + [var]
        genParity(left, 0)
        genParityChain(right, phase)

# Generate atMost-k ladder and assert that it holds
# Only enforce one side of constraints (Plaisted-Greenbaum)
def enforceAmk(vars, k):
    m = len(vars)
    # Assume 0 <= k < m
    # Lits of variables encoding counts.
    # entry (i,j) gives variable for vars[:i] having at most j 1's
    slist = [str(v) for v in vars]
    if verbose:
        cwriter.doComment("Encoding of at-most-%d over variables [%s]" % (k, ", ".join(slist)))
    c = {}
    # Count = 0
    c[(0,0)] = -vars[0]
    cwriter.doComment("Encode at-most-0 conditions")
    for i in range(1, m-k):
        here = cwriter.newVariable()
        if verbose:
            cwriter.doComment("Variable %d encodes that vars 0..%d have at most %d 1's" % (here, i,0))
        c[(i,0)] = here
        local = vars[i]
        prev = c[(i-1,0)]
        cwriter.doClause([-local, -here])
        cwriter.doClause([prev, -here])
        # Don't need constraint:
        if not optimize:
            cwriter.doClause([local, -prev, here])
    for j in range(1, k+1):
        if verbose:
            cwriter.doComment("Encode at-most-%d conditions" % j)
        i = j
        here = cwriter.newVariable()
        if verbose:
            cwriter.doComment("Variable %d encodes that vars 0..%d have at most %d 1's" % (here,i,j))
        c[(i,j)] = here
        local = vars[i]
        prevJm1 = c[(i-1,j-1)]
        cwriter.doClause([-local, prevJm1, -here])
        # Don't need constraint:
        if not optimize:
            cwriter.doClause([local, here])
            cwriter.doClause([-prevJm1, here])
        for i in range(j+1, m+j-k):
            here = cwriter.newVariable()
            if verbose:
                cwriter.doComment("Variable %d encodes that vars 0..%d have at most %d 1's" % (here,i,j))
            c[(i,j)] = here
            local = vars[i]
            prevJ = c[(i-1,j)]
            prevJm1 = c[(i-1,j-1)]
            cwriter.doClause([prevJ, -here])
            cwriter.doClause([-local, prevJm1, -here])
            # Don't need constraint:
            if not optimize:
                cwriter.doClause([-prevJm1, -prevJ, here])
                cwriter.doClause([local, -prevJ, here])
    topVar = c[(m-1,k)]
    if verbose:
        cwriter.doComment("Assert that at-most-%d condition holds" % numTolerated)
    cwriter.doClause([topVar])


def initData(fixed):
    global solutionBits, corruptionBits, sampleBitSet, parityBits, phaseBits
    if fixed:
        solutionBits = [i%2 for i in range(numVariables)]
        corruptionBits = [1 if i < numCorrupt else 0 for i in range(numSamples)]
    else:
        solutionBits = randomBits(numVariables)
        corruptionVars = random.sample(range(numSamples), numCorrupt)
        corruptionBits = [1 if i in corruptionVars else 0 for i in range(numSamples)]

    sampleBitSet = []
    parityBits = []
    phaseBits = []

    for i in range(numSamples):
        sbits = randomBits(numVariables)
        sampleBitSet.append(sbits)
        samples = [solve & sample for solve, sample in zip(solutionBits, sbits)]
        parity = vectorParity(samples)
        parityBits.append(parity)
        phase = parity ^ corruptionBits[i]
        phaseBits.append(phase)


def initVariables():
    global solutionVariables, corruptionVariables
    solutionVariables = cwriter.newVariables(numVariables)
    corruptionVariables = cwriter.newVariables(numSamples)

def document(anonymous):
    cwriter.doComment("Parity sampling problem.  n=%d, m=%d, k=%d, t=%d, seed=%d" % (numVariables, numSamples, numCorrupt, numTolerated, seed))
    print("c Parity sampling problem.  n=%d, m=%d, k=%d, t=%d, seed=%d" % (numVariables, numSamples, numCorrupt, numTolerated, seed))
    if not anonymous:
        cwriter.doComment("Target Solution: %s" % vectorString(solutionBits))
        print("c Target Solution: %s" % vectorString(solutionBits))
    cwriter.doComment("Solution variables: %s" % vectorString(solutionVariables))
    print("c Solution variables: %s" % vectorString(solutionVariables))
    if not anonymous:
        cwriter.doComment("Corrupted samples: %s" % vectorString(corruptionBits))
        print("c Corrupted samples: %s" % vectorString(corruptionBits))
    cwriter.doComment("Corruption variables: %s" % vectorString(corruptionVariables))
    print("c Corruption variables: %s" % vectorString(corruptionVariables))
    if not anonymous:
        for i in range(numSamples):
            if corruptionBits[i]:
                cwriter.doComment("Sample #%.2d: %s | %d (Corrupted)" % (i+1, vectorString(sampleBitSet[i]), phaseBits[i]))
                print("c Sample #%.2d: %s | %d (Corrupted)" % (i+1, vectorString(sampleBitSet[i]), phaseBits[i]))
            else:
                cwriter.doComment("Sample #%.2d: %s | %d" % (i+1, vectorString(sampleBitSet[i]), phaseBits[i]))
                print("c Sample #%.2d: %s | %d" % (i+1, vectorString(sampleBitSet[i]), phaseBits[i]))


def generate():
    # Generate Parity Computations
    for i in range(numSamples):
        vars = [corruptionVariables[i]] + [solutionVariables[j] for j in range(numVariables) if sampleBitSet[i][j] == 1]
        phase = phaseBits[i]
        cstring = "Y" if corruptionBits[i] == 1 else "N"
        if verbose:
            cwriter.doComment("Sample #%d.  Vars = %s.  Phase = %d (Corrupted: %s)" % (i, vectorString(vars), phase, cstring))
        genParityChain(vars, phase)
    # Generate at-most-k
    enforceAmk(corruptionVariables, numTolerated)


def run(name, args):
    global verbose, optimize, numVariables, numSamples, numCorrupt, numTolerated, seed
    global cwriter
    fixed = False
    anonymous = False
    numSamples = -1
    numCorrupt = -1
    numTolerated = -1
    optlist, args = getopt.getopt(args, "hvfOan:m:k:t:s:")
    for (opt, val) in optlist:
        if opt == '-h':
            usage(name)
            return
        elif opt == '-v':
            verbose = True
        elif opt == '-O':
            optimize = True
        elif opt == '-f':
            fixed = True
        elif opt == '-a':
            anonymous = True
        elif opt == '-n':
            numVariables = int(val)
        elif opt == '-m':
            numSamples = int(val)
        elif opt == '-k':
            numCorrupt = int(val)
        elif opt == '-t':
            numTolerated = int(val)
        elif opt == '-s':
            seed = int(val)
    if numSamples < 0:
        numSamples = 2*numVariables
    if numCorrupt < 0:
        # Crawford states that he used k = m / 8 for his benchmarks.
        # This seems to be a good number.  Larger values admit too many solutions
        numCorrupt = numSamples // 8
    if numTolerated < 0:
        numTolerated = numCorrupt

    base = "mdparity"
    if fixed:
        base += "-fixed"
    root = "%s-n%d-k%d-t%d-s%d" % (base, numVariables, numCorrupt, numTolerated, seed)
    random.seed(seed)
    initData(fixed)

    cwriter = writer.LazyCnfWriter(root, verbose=False)
    initVariables()
    document(anonymous)
    generate()
    cwriter.finish()

if __name__ == "__main__":
    run(sys.argv[0], sys.argv[1:])

