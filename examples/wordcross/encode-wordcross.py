#!/usr/bin/python3

import sys

def wordcross(words, rows, cols):
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

    for word in words:
        for r in range(rows):
            for c in range(cols-len(word)+1):
                print(" ".join(horizontal_placement(word, r, c)))
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
    try:
        assert(len(sys.argv) == 4)
        wordfile = sys.argv[1]
        rows = int(sys.argv[2])
        assert(rows > 0)
        cols = int(sys.argv[3])
        assert(cols > 0)
    except:
        print('Usage: "{} wordfile rows cols"'.format(sys.argv[0]))
        sys.exit(-1)
    words = [w.strip() for w in open(wordfile) if len(w.strip()) > 0]
    wordcross(words, rows, cols)