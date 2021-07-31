#!/usr/bin/python3
import re
import sys

def parse_solution(ss):
    assert(len(ss) == 81), "Input must be 81 characters long"
    sol = []
    while len(ss) > 0:
        sol.append([s for s in ss[:9]])
        ss = ss[9:]
    return sol

def unparse_solution(sol):
    return ''.join(''.join(row) for row in sol)

def decode(sol):
    s = None
    for line in sys.stdin:
        line = line.rstrip()
        if re.match('.*Solution:', line):
            if s is not None: yield unparse_solution(s)
            s = [row[:] for row in sol]
            continue
        m = re.match('  \\d+: p(\\d)(\\d) r\\d(\\d)', line)
        if m is None: continue
        r, c, val = m.groups()
        s[int(r)][int(c)] = val
    if s is not None: yield unparse_solution(s)

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2), "Wrong number of parameters"
        sol = parse_solution(sys.argv[1])
        for ss in decode(sol):
            print(ss)
    except AssertionError:
        print('Usage: "encode-sudoku.py <partial-solution>"')
        raise
