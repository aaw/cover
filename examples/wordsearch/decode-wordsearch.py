#!/usr/bin/python3
import re
import sys

def print_board(b):
    return '\n'.join(''.join(row) for row in b)

def decode(rows, cols):
    b = None
    for line in sys.stdin:
        line = line.rstrip()
        if re.match('.*Solution:', line):
            if b is not None: yield print_board(b)
            b = [['_']*cols for row in range(rows)]
            continue
        m = re.match('  \\d+: (\\w+)', line)
        if m is None: continue
        word = m.groups()[0]
        cs = [(int(x),int(y)) for x,y in re.findall('\((\\d+),(\\d+)\)', line)]
        for coord, letter in zip(cs,word):
            b[coord[0]][coord[1]] = letter
    if b is not None: yield print_board(b)

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 3), "Wrong number of flags"
        rows = int(sys.argv[1])
        cols = int(sys.argv[2])
        for ss in decode(rows, cols):
            print(ss)
            print()
    except AssertionError:
        print('Usage: {} rows cols'.format(sys.argv[0]))
        raise
