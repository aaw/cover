#!/usr/bin/python3

# Solutions to Robert Wainwright's partridge puzzle: Can one 1x1 square, two 2x2
# squares, three 3x3 squares, ..., n nxn squares be tiled perfectly into an NxN
# square, where N=1+2+3+...+n?

import sys

def partridge(n):
    N = n*(n+1)//2
    tiles = ["#{}[{}:{}]".format(x,x,x) for x in range(1,n+1)]
    positions = ["({},{})".format(i,j) for i in range(N) for j in range(N)]
    print(" ".join(tiles + positions))
    for tnum in range(1,n+1):
        for r in range(N-tnum+1):
            for c in range(N-tnum+1):
                print("#{} ".format(tnum) +
                      " ".join("({},{})".format(r+rr,c+cc) \
                               for rr in range(tnum) for cc in range(tnum)))

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2)
        n = int(sys.argv[1])
        assert(n > 1)
    except:
        print('Usage: {} n"'.format(sys.argv[0]))
        sys.exit(-1)
    print("// Partridge problem solutions for n = {}.".format(n))
    print("// Items #k for k in [1,n] represent the choice of a kxk tile.")
    print("// Items (i,j) represent positions.")
    print("// Options of the form \"#k (i,j) (i+1,j) ...\" represent placing ")
    print("// the tile k on a particular position on the board.")
    print()
    partridge(n)
