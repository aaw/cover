#!/usr/bin/python3

import re
import sys

patterns = {
    (0,0,
     0,1): "\u250c",  # ┌
    (0,0,
     1,0): "\u2510",  # ┐
    (0,0,
     1,1): "\u2500",  # ─
    (0,0,
     1,2): "\u252c",  # ┬
    (0,1,
     0,0): "\u2514",  # └
    (0,1,
     0,1): "\u2502",  # │
    (0,1,
     0,2): "\u251c",  # ├
    (0,1,
     1,1): "\u2518",  # ┘
    (0,1,
     2,1): "\u2524",  # ┤
    (0,1,
     2,2): "\u2534",  # ┴
    (0,1,
     2,3): "\u253c",  # ┼
}

lower_right_borders = set([
    (0,1,1,1), # ┘
    (0,1,2,1), # ┤
    (0,1,2,2), # ┴
    (0,1,2,3), # ┼
])

# Sizing of blocks in terms of box-drawing characters above:
#
# ┌─┐   1 = 3x2
# └─┘
#
# ┌───┐ 2 = 5x3
# │  2│
# └───┘
#
# ┌─────┐ 3 = 7x4
# │     │
# │    3│
# └─────┘
#
# So an nxn block is represented by 2n+1 x n+1 box drawing characters.

def canonicalize_box(box):
    cmap = {}
    cbox = [-1]*4
    cid = 0
    for i,val in enumerate(box):
        if cmap.get(val) is None:
            cmap[val] = cid
            cid += 1
        cbox[i] = cmap[val]
    return tuple(cbox)

def draw(box):
    global patterns
    return patterns.get(canonicalize_box(box))

def is_border(box):
    global lower_right_borders
    return canonicalize_box(box) in lower_right_borders

def pretty_print_board(b):
    for r in range(len(b)-1):
        for c in range(len(b[0])-1):
            coord = (b[r][c][0],b[r][c+1][0],b[r+1][c][0],b[r+1][c+1][0])
            ch = draw(coord)
            if ch is None:
                if r < len(b)-1 and c < len(b[0])-1:
                    border = (b[r+1][c+1][0],b[r+1][c+2][0],
                              b[r+2][c+1][0],b[r+2][c+2][0])
                    if is_border(border):
                        print(b[r][c][1], end='')
                    else:
                        print(' ', end='')
            else:
                print(ch, end='')
        print('')

def stretch_coords(x,y,size):
    x, y = x+1, 2*y+1
    return [(x+xx,y+yy) for xx in range(size) for yy in range(2*size)]

def decode(n):
    b = None
    N = n*(n+1)//2
    i = 0
    for line in sys.stdin:
        line = line.rstrip()
        if re.match('.*Solution:', line):
            if b is not None: yield b
            i = 1
            b = [[(0,0)]*(2*N+2) for row in range(N+2)]
            continue
        m = re.match('  \\d+: #(\\d+)', line)
        if m is None: continue
        size = int(m.groups()[0])
        cs = [(int(x),int(y)) for x,y in re.findall('\((\\d+),(\\d+)\)', line)]
        mincoord = min(cs)
        maxcoord = max(cs)
        cs = stretch_coords(mincoord[0], mincoord[1], size)
        for coord in cs:
            b[coord[0]][coord[1]] = (i,size)
        i += 1
    if b is not None: yield b

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2), "Wrong number of flags"
        n = int(sys.argv[1])
        for b in decode(n):
            pretty_print_board(b)
            print()
    except AssertionError:
        print('Usage: {} rows cols'.format(sys.argv[0]))
        raise
