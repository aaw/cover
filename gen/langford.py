#!/usr/bin/python3
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
    langford(n)
