#!/usr/bin/python3

import sys

def wordsearch(words, rows, cols):
    cstr = dict(((r,c), "({},{})".format(r,c)) \
                 for r in range(rows) for c in range(cols))
    print(" ".join(words + ['|'] + [v for v in cstr.values()]))
    for word in words:
        for r in range(rows):
            for c in range(cols):
                for w in (word, ''.join(reversed(word))):
                    if c + len(word) <= cols:
                        # Generate horizontal placement
                        os = ["{}:{}".format(coord,color) for coord,color in \
                              zip((cstr[(r,c+i)] for i in range(len(w))), w)]
                        print(" ".join([word] + os))
                    if r + len(word) <= rows:
                        # Generate vertical placement
                        os = ["{}:{}".format(coord,color) for coord,color in \
                              zip((cstr[(r+i,c)] for i in range(len(w))), w)]
                        print(" ".join([word] + os))
                    if r + len(word) <= rows and c - len(word) >= 0:
                        # Generate upward diagonal placement
                        os = ["{}:{}".format(coord,color) for coord,color in \
                              zip((cstr[(r+i,c-i)] for i in range(len(w))), w)]
                    if r + len(word) <= rows and c + len(word) <= cols:
                        # Generate downward diagonal placement
                        os = ["{}:{}".format(coord,color) for coord,color in \
                              zip((cstr[(r+i,c+i)] for i in range(len(w))), w)]

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 4)
        wordfile = sys.argv[1]
        rows = int(sys.argv[2])
        assert(rows > 0)
        cols = int(sys.argv[3])
        assert(cols > 0)
    except:
        print('Usage: "encode-wordsearch.py wordfile rows cols"')
        sys.exit(-1)
    words = [w.strip() for w in open(wordfile) if len(w.strip()) > 0]
    wordsearch(words, rows, cols)
