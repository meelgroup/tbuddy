#!/usr/bin/python

# Step through FRAT file and find instances where clauses are not
# properly handled.  Most commonly, these are when a clause is defined
# via either an 'o' or a 'a' command, but not either deleted via 'd'
# command or listed at the end via a 'f' command.

# Usage: ./frat-track.py < file.frat

import sys

err_max = 50
err_count = 0

add_commands = ['a', 'o']
delete_commands = ['d', 'f']
reloc_commands = ['r']

# Mapping from clause Id to its count
clause_tracker = {}


line_count = 0

cmd_counters = {}

for line in sys.stdin:
    line_count += 1
    fields = line.split()
    if len(fields) < 2 or fields[0] == 'c':
        continue
    cmd = fields[0]
    if cmd in cmd_counters:
        cmd_counters[cmd] += 1
    else:
        cmd_counters[cmd] = 1
    id = int(fields[1])
    if cmd in add_commands:
        if id in clause_tracker:
            clause_tracker[id] += 1
        else:
            clause_tracker[id] = 1
    elif cmd in delete_commands:
        if id not in clause_tracker:
            err_count += 1
            if err_count <= err_max:
                print("Line #%d.  Clause %d not previously defined" % (line_count, id))
        elif clause_tracker[id] == 0:
            err_count += 1
            if err_count <= err_max:
                print("Line #%d.  Clause %d already accounted for" % (line_count, id))
        else:
            clause_tracker[id] -= 1
    elif cmd in reloc_commands:
        id2 = int(fields[2])
        clause_tracker[id] -= 1
        if id2 in clause_tracker and clause_tracker[id] > 0:
            err_count += 1
            if err_count <= err_max:
                print("Line #%d. Clause %d is reloced to ID %d but %d is already used" % (line_count, id, id2, id2))
        clause_tracker[id2] = 1
    else:
        print("Line #%d.  Unknown command '%s'" % (line_count, cmd))

ids = sorted(clause_tracker.keys())

left_count = 0

for id in ids:
    if clause_tracker[id] > 0:
        err_count += 1
        left_count += 1
        if err_count <= err_max:
            print("Clause %d created but not deleted" % id)

if left_count > 0:
    print("%d clauses unaccounted for" % left_count)    
else:
    print("All clauses accounted for")    
if err_count > 0:
    print("%d errors encountered" % err_count)

print("Command types totals:")
for cmd in sorted(cmd_counters.keys()):
    print("   Command:%s %d" % (cmd, cmd_counters[cmd]))
