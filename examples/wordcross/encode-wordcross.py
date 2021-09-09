#!/usr/bin/python3

import argparse
import sys

def wordcross(words, args):
    rows, cols = args.rows, args.cols
    ids = dict((w, "#{}".format(i+1)) for i,w in enumerate(words))
    hs = dict(((r,c), "H({},{})".format(r,c)) \
              for r in range(rows) for c in range(cols-1))
    vs = dict(((r,c), "V({},{})".format(r,c)) \
              for r in range(rows-1) for c in range(cols))
    coords = dict(((r,c), "({},{})".format(r,c)) \
                  for r in range(rows) for c in range(cols))
    pcoords = dict(((r,c), "({},{})'".format(r,c)) \
                   for r in range(rows) for c in range(cols))

    def horizontal_placement(word, r, c):
        p = [ids[word]]
        if c > 0:
            p.append("{}:.".format(coords[(r,c-1)]))
            p.append("{}:0".format(pcoords[(r,c-1)]))
        for i,ch in enumerate(word):
            p.append("{}:{}".format(coords[(r,c+i)], ch))
            if i != len(word)-1: p.append(hs[r,c+i])
            p.append("{}:1".format(pcoords[(r,c+i)]))
        if c != cols-len(word):
            p.append("{}:.".format(coords[(r,c+len(word))]))
            p.append("{}:0".format(pcoords[(r,c+len(word))]))
        return p

    def vertical_placement(word, r, c):
        p = [ids[word]]
        if r > 0:
            p.append("{}:.".format(coords[(r-1,c)]))
            p.append("{}:0".format(pcoords[(r-1,c)]))
        for i,ch in enumerate(word):
            p.append("{}:{}".format(coords[(r+i,c)], ch))
            if i != len(word)-1: p.append(vs[r+i,c])
            p.append("{}:1".format(pcoords[(r+i,c)]))
        if r != rows-len(word):
            p.append("{}:.".format(coords[(r+len(word),c)]))
            p.append("{}:0".format(pcoords[(r+len(word),c)]))
        return p

    primary = list(ids.values()) + list(hs.values()) + list(vs.values())
    secondary = list(coords.values()) + list(pcoords.values())
    print(" ".join(primary + ['|'] + secondary))

    for i, word in enumerate(words):
        for r in range(rows):
            for c in range(cols-len(word)+1):
                print(" ".join(horizontal_placement(word, r, c)))
        # Symmetry-breaking: omit vertical placement of first word only.
        if args.break_symmetry and rows == cols and i == 0: continue
        for r in range(rows-len(word)+1):
            for c in range(cols):
                print(" ".join(vertical_placement(word, r, c)))

    for r in range(rows):
        for c in range(cols-1):
            print("{} {}:0 {}:1 {}:.".format(
                hs[(r,c)], pcoords[(r,c)], pcoords[(r,c+1)], coords[(r,c)]))
            print("{} {}:1 {}:0 {}:.".format(
                hs[(r,c)], pcoords[(r,c)], pcoords[(r,c+1)], coords[(r,c+1)]))
            print("{} {}:0 {}:0 {}:. {}:.".format(
                hs[(r,c)], pcoords[(r,c)], pcoords[(r,c+1)], coords[(r,c)],
                coords[(r,c+1)]))

    for r in range(rows-1):
        for c in range(cols):
            print("{} {}:0 {}:1 {}:.".format(
                vs[(r,c)], pcoords[(r,c)], pcoords[(r+1,c)], coords[(r,c)]))
            print("{} {}:1 {}:0 {}:.".format(
                vs[(r,c)], pcoords[(r,c)], pcoords[(r+1,c)], coords[(r+1,c)]))
            print("{} {}:0 {}:0 {}:. {}:.".format(
                vs[(r,c)], pcoords[(r,c)], pcoords[(r+1,c)], coords[(r,c)],
                coords[(r+1,c)]))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Encode a wordcross problem as XCC")
    parser.add_argument('wordfile', type=str, help='input words, one per line')
    parser.add_argument('rows', type=int, help='number of rows')
    parser.add_argument('cols', type=int, help='number of columns')
    parser.add_argument('--break_symmetry',
                        action='store_true',
                        help='break symmetry when rows=cols')
    args = parser.parse_args()
    words = [w.strip() for w in open(args.wordfile) if len(w.strip()) > 0]
    wordcross(words, args)
