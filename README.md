cover
=====

Solvers for the [exact set cover](https://en.wikipedia.org/wiki/Exact_cover) problem and variants. All
solvers are translations of Don Knuth's [Dancing Links](https://en.wikipedia.org/wiki/Dancing_Links)
programs described in The Art of Computer Programming, Volume 4, Fascicle 5.

Exact set cover problems consist of a set of _items_ and a list of _options_. Each option is a
subset of the items. The goal is to find a set of options that form a partition of the items:
each pair of options selected must be pairwise disjoint and the union of all selected options
must be the entire set of items.

For example, if the items are _{a,b,c,d}_ and the list of options are:

    * _{a,b}_
    * _{c,d}_
    * _{a,c}_
    * _{b,d}_
    * _{a,d}_

then either

    * _{a,b}_ and _{c,d}_ or
    * _{a,c}_ and _{b,d}_

are the only two solutions to the exact set cover problem.

The solvers in this repo enumerate all solutions to a given set cover problem.

Example usage
-------------

Input files for all solvers must be newline-delimited and consist of a line of items
followed by one or more lines of options. Each line of items or options is space-delimited.

An input file for the simple example in the previous section would look like:

```
a b c d
a b
c d
a c
b d
a d
```

If you save this file to `input.xc` and run a solver on it, you should see:

```
$ ./bin/xc input.xc
[src/xc.cc:102] Parsed 4 items (4 primary)
[src/xc.cc:159] Parsed 5 options
[src/xc.cc:256] Solution:
  1: a b
  2: c d

[src/xc.cc:256] Solution:
  4: b d
  3: a c

counter: [solutions] = 2
```

More examples
-------------

A few Python scripts in this repo generate input files for interesting applications of exact set cover:

   * [gen/langford.py](gen/langford.py): Generates [Langford pairings](https://en.wikipedia.org/wiki/Langford_pairing).
   * [gen/nqueens.py](gen/nqueens.py): Generates solutions to the [N-queens problem](https://en.wikipedia.org/wiki/Eight_queens_puzzle)
   * [gen/pentomino.py](gen/pentomino.py): Generates rectangular tilings of [Pentominoes](https://en.wikipedia.org/wiki/Pentomino) using Conway's O-Z labeling. Each pentomino is used exactly once.

Some examples benefit from filtering or transforming the output in simple ways. For these, there are
scripts that wrap the call to the exact set cover solver:

   * [examples/sudoku](examples/sudoku): A [Sudoku](https://en.wikipedia.org/wiki/Sudoku) solver.
   * [examples/wordsearch](examples/wordsearch): A [word search](https://en.wikipedia.org/wiki/Word_search) generator.
   * [examples/wordcross](examples/wordcross): A word cross generator: generates [diagrams like this](https://digitalcommons.butler.edu/cgi/viewcontent.cgi?referer=&httpsredir=1&article=3847&context=wordways).
   * [examples/partridge](examples/partridge): A solver for Robert T. Wainwright's [Partridge Puzzle](https://www.mathpuzzle.com/partridge.html).

All of these examples appear in one form or another in The Art of Computer Programming Volume 4, Fascicle 5.

Building
--------

You'll need `git` to clone this repo, `g++` and `make` to build and `bash` and `python3` to run
instance generators and examples.

On a debian-based Linux distribution, you can make sure you have everything you need by
running:

    apt-get update && apt-get install bash build-essential git python3

Next, clone this repo:

    git clone git@github.com:aaw/cover.git

And type `make`.

Advanced features
-----------------
