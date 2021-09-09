#!/usr/bin/python3

import argparse
import re
import sys

def print_board(b):
    return '\n'.join(''.join(row) for row in b)

def traverse(b,r,c,visited):
    if r < 0 or r >= len(b): return 0
    if c < 0 or c >= len(b[r]): return 0
    if visited[r][c]: return 0
    visited[r][c] = True
    if not b[r][c].isalpha(): return 0
    traverse(b,r,c+1,visited)
    traverse(b,r,c-1,visited)
    traverse(b,r+1,c,visited)
    traverse(b,r-1,c,visited)
    return 1

def components(b):
    n = 0
    visited = [[False for c in r] for r in b]
    for r in range(len(b)):
        for c in range(len(b[r])):
            n += traverse(b,r,c,visited)
    return n

def decode(rows, cols):
    b = None
    for line in sys.stdin:
        line = line.rstrip()
        if re.match('.*Solution:', line):
            if b is not None: yield b
            b = [['.']*cols for row in range(rows)]
            continue
        m = re.match('  \\d+: .*', line)
        if m is None: continue
        cs = [(int(x),int(y),z) for x,y,z in \
              re.findall('\((\\d+),(\\d+)\):(\\w+)', line)]
        for coord in cs:
            b[coord[0]][coord[1]] = coord[2]
    if b is not None: yield b

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Decode a wordcross problem from XCC output")
    parser.add_argument('rows', type=int, help='number of rows')
    parser.add_argument('cols', type=int, help='number of columns')
    parser.add_argument('--only_connected', action='store_true',
                        help='only output connected solutions')
    parser.add_argument('--deduplicate', action='store_true',
                        help='only output unique solutions')
    args = parser.parse_args()

    s = set()
    total = 0
    for b in decode(args.rows, args.cols):
        if args.only_connected and components(b) > 1: continue
        bs = print_board(b)
        if args.deduplicate:
            if bs in s: continue
            s.add(bs)
        print(bs)
        total += 1
        print()
    print('Total: %s' % total)
