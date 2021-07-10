#!/usr/bin/python3
import sys

def parse_solution(ss):
    assert(len(ss) == 81), "Input must be 81 characters long"
    def tonum(s):
        if s == '.': return 0
        return int(s)
    sol = []
    while len(ss) > 0:
        sol.append([tonum(s) for s in ss[:9]])
        ss = ss[9:]
    return sol

def all_items():
    items = set()
    for r in range(9):
        for c in range(9):
            items.add('p{}{}'.format(r,c))
    for i in range(9):
        for k in range(1,10):
            items.add('r{}{}'.format(i,k))
            items.add('c{}{}'.format(i,k))
            items.add('b{}{}'.format(i,k))
    return items

def all_options():
    options = []
    for i in range(9):
        for j in range(9):
            for k in range(1,10):
                x = 3 * (i // 3) + (j // 3)
                options.append(('p{}{}'.format(i,j),
                                'r{}{}'.format(i,k),
                                'c{}{}'.format(j,k),
                                'b{}{}'.format(x,k)))
    return options

def encode(sol):
    items = all_items()
    options = all_options()
    for r, row in enumerate(sol):
        for c, val in enumerate(row):
            if val == 0: continue
            x = 3 * (r // 3) + (c // 3)
            items.remove('p{}{}'.format(r,c))
            items.remove('r{}{}'.format(r,val))
            items.remove('c{}{}'.format(c,val))
            items.remove('b{}{}'.format(x,val))
    options = [o for o in options if all(oo in items for oo in o)]
    print(' '.join(items))
    for o in options:
        print(' '.join(o))

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2), "Wrong number of parameters"
        sol = parse_solution(sys.argv[1])
        encode(sol)
    except AssertionError:
        print('Usage: "encode-sudoku.py <partial-solution>"')
        raise
