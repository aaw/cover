#!/usr/bin/python3
import sys

O = ((1,1,1,1,1),)
P = ((1,1),
     (1,1),
     (1,0),)
Q = ((1,1,1,1),
     (0,0,0,1),)
R = ((0,1,1),
     (1,1,0),
     (0,1,0),)
S = ((0,0,1,1),
     (1,1,1,0),)
T = ((1,1,1),
     (0,1,0),
     (0,1,0),)
U = ((1,0,1),
     (1,1,1),)
V = ((1,0,0),
     (1,0,0),
     (1,1,1),)
W = ((1,0,0),
     (1,1,0),
     (0,1,1),)
X = ((0,1,0),
     (1,1,1),
     (0,1,0),)
Y = ((0,0,1,0),
     (1,1,1,1),)
Z = ((1,1,0),
     (0,1,0),
     (0,1,1),)
pentominoes = {'O':O, 'P':P, 'Q':Q, 'R':R,
               'S':S, 'T':T, 'U':U, 'V':V,
               'W':W, 'X':X, 'Y':Y, 'Z':Z}

def flip(p):
    return tuple(tuple(i for i in reversed(row)) for row in p)

def rotate(p):
    return tuple(tuple(x) for x in zip(*p))[::-1]

assert(flip(X) == X)
assert(flip(Y) != Y)
assert(rotate(X) == X)
assert(rotate(Y) != Y)
assert(rotate(T) != T)
assert(rotate(rotate(T)) != T)
assert(rotate(rotate(rotate(T))) != T)
assert(rotate(rotate(rotate(rotate(T)))) == T)
assert(all(flip(flip(p)) == p for p in pentominoes.values()))
assert(all(rotate(rotate(rotate(rotate(p)))) == p
           for p in pentominoes.values()))

def translations(p):
    s = set()
    for q in (p, flip(p)):
        s.add(q)
        s.add(rotate(q))
        s.add(rotate(rotate(q)))
        s.add(rotate(rotate(rotate(q))))
    return s

def all_positions(p, nrows, ncols):
    for q in translations(p):
        qr, qc = len(q), len(q[0])
        for r in range(nrows-qr+1):
            for c in range(ncols-qc+1):
                yield tuple((r+i,c+j)
                            for i in range(qr) for j in range(qc)
                            if q[i][j] == 1)

def pstr(pos):
    return "({},{})".format(*pos)

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 3)
        nr = int(sys.argv[1])
        nc = int(sys.argv[2])
        assert(nr > 1)
        assert(nc > 1)
    except:
        print('Usage: "pentomino.py r c"')
        sys.exit(-1)
    print(" ".join([k for k in pentominoes] +
                   [pstr((r,c)) for r in range(nr) for c in range(nc)]))
    for label, p in pentominoes.items():
        for pos in all_positions(p, nr, nc):
            print(" ".join([label] + list(pstr(t) for t in pos)))
