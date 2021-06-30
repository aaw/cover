#!/usr/bin/python3
import sys

def nqueens(n):
    rows = ["r{}".format(i) for i in range(1,n+1)]
    cols = ["c{}".format(i) for i in range(1,n+1)]
    udiag = set("a{}".format(i+j) for i in range(1,n+1) for j in range(1,n+1))
    ddiag = set("b{}".format(i-j+n) for i in range(1,n+1) for j in range(1,n+1))
    print(" ".join(rows + cols) + " | " + " ".join(udiag | ddiag))
    for r in range(1,n+1):
        for c in range(1,n+1):
            print("r{} c{} a{} b{}".format(r,c,r+c,r-c+n))

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2)
        n = int(sys.argv[1])
        assert(n > 1)
    except:
        print('Usage: "nqueens.py n"')
        sys.exit(-1)
    nqueens(n)
