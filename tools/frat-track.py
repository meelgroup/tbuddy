#!/usr/bin/python

# Step through FRAT file and find instances where clauses are not
# properly handled.  Most commonly, these are when a clause is defined
# via either an 'o' or a 'a' command, but not either deleted via 'd'
# command or listed at the end via a 'f' command.

# Usage: ./frat-track.py < file.frat

import sys


add_commands = ['a', 'o']
delete_commands = ['d', 'f']
reloc_commands = ['r']

# Mapping from clause Id to its count
clause_tracker = {}


line_count = 0
for line in sys.stdin:
    line_count += 1
    fields = line.split()
    if len(fields) < 2 or fields[0] == 'c':
        continue
    cmd = fields[0]
    id = int(fields[1])
    if cmd in add_commands:
        if id in clause_tracker:
            clause_tracker[id] += 1
        else:
            clause_tracker[id] = 1
    elif cmd in delete_commands:
        if id not in clause_tracker:
            print("Line #%d.  Clause %d not previously defined" % (line_count, id))
        elif clause_tracker[id] == 0:
            print("Line #%d.  Clause %d already accounted for" % (line_count, id))
        else:
            clause_tracker[id] -= 1
    elif cmd in reloc_commands:
        id2 = int(fields[2])
        clause_tracker[id] -= 1
        if id2 in clause_tracker:
            print("Line #%d. Clause %d is reloced to ID %d but %d is already used" % (line_count, id, id2, id2))
        clause_tracker[id2] = 1
    else:
        print("Line #%d.  Unknown command '%s'" % (line_count, cmd))

ids = sorted(clause_tracker.keys())

left_count = 0

for id in ids:
    if clause_tracker[id] > 0:
        left_count += 1
        print("Clause %d created but not deleted" % id)

print("%d clauses unaccounted for" % left_count)    

        
    
