Wordcross
=========

The script in this directory creates wordcross diagrams: rectangles containing a set of words
that are all rookwise connected to each other but aren't adjacent to each other unless they
cross.

One example of a wordcross puzzle is the "Presidential Rectangle", explored by a series of
articles over a few decades in Word Ways:

* [We must all hang together](https://digitalcommons.butler.edu/wordways/vol9/iss1/3/)
* [A Presidential Rectangle](https://digitalcommons.butler.edu/wordways/vol9/iss2/4/)
* [A New Presidential Rectangle](https://digitalcommons.butler.edu/wordways/vol26/iss2/5/)
* [A Smaller Presidential Rectangle](https://digitalcommons.butler.edu/wordways/vol26/iss3/14/)
* [An Improved Presidential Rectangle](https://digitalcommons.butler.edu/wordways/vol26/iss4/6/)
* [The Ultimate Presidential Rectangle?](https://digitalcommons.butler.edu/wordways/vol27/iss1/18/)

This directory has a file `numbers.txt` with the numbers ZERO through TEN. You can generate
all 8-by-8 wordcrosses containing these numbers by running `create-wordcross.sh numbers.txt 8 8`,
which should generate rectangles like:

```
...F..F.
ZERO.SIX
.I.U..V.
.G.R.TEN
.H...H.I
.TWO.R.N
...N.E.E
..SEVEN.

..FIVE..
TWO..I..
..U..G..
ZERO.H..
...N.TEN
SEVEN..I
I......N
X..THREE
```

Unfortunately, the presidential rectangle is a bit too challenging for the solvers in this repo for any
recent list of presidents.