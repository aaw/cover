#!/usr/bin/python3

# From https://en.wikipedia.org/wiki/Langford_pairing:
# A Langford pairing is a permutation of the sequence of 2n numbers
# 1, 1, 2, 2, ..., n, n in which the two 1s are one unit apart, the
# two 2s are two units apart, and more generally the two copies of
# each number k are k units apart.

# Langford pairings are only possible when n = 0 mod 4 or n = 3 mod 4.

import sys

def langford(n):
    nums = [str(x) for x in range(1,n+1)]
    labels = ["s{}".format(x) for x in range(1,2*n+1)]
    print(" ".join(nums + labels))
    for n in nums:
        nn = int(n)
        for i in range(len(labels)-nn-1):
            print("{} {} {}".format(nn, labels[i], labels[i+nn+1]))

if __name__ == '__main__':
    try:
        assert(len(sys.argv) == 2)
        n = int(sys.argv[1])
        assert(n > 1)
    except:
        print('Usage: "langford.py n"')
        sys.exit(-1)
    print("# Langford pairings for n = {}.".format(n))
    print("# Items 1 ... n represent the choice of a number.")
    print("# Items s1 ... s2n represent the positions of a number.")
    print("# Options of the form \"1 s1 s3\" represent valid")
    print("# placements: \"1 is placed in position 1 and 3\"")
    print()
    langford(n)
