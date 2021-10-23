# cover

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

## Example usage

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

## More examples

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

## Building

You'll need `git` to clone this repo, `g++` and `make` to build and `bash` and `python3` to run
instance generators and examples. Some examples needs `wget` to download external input files.

On a debian-based Linux distribution, you can make sure you have everything you need by
running:

    apt-get update && apt-get install bash build-essential git python3 wget

Next, clone this repo:

    git clone git@github.com:aaw/cover.git

And type `make`. This produces three binaries:

   * `xc`: An exact set cover solver
   * `xcc`: A solver supporting color constraints
   * `mcc`: A solver supporting multiplicities and color constraints

## Input format

The three solvers in this repo each support slightly different input formats, but
all support the basic newline-delimited items-followed-by-options format described
above.

In all three solvers, anything after two forward slashes (`//`) is considered a
comment and ignored. A backslash preceded by a space (` \`) can be used for line
continuation to split a long line into multiple lines in the input. See [test/simple_4.xc](test/simple_4.xc)
for an extended example of comments and line continuations.

### Items

Items can be any string with the following restrictions:

   * Can't contain spaces (these are used to delimit items and options)
   * Can't contain pipes (`|`)
   * Can't contain colons (`:`)
   * Can't contain brackets (`[`, `]`)

Pipes, colons, and brackets are all used for special input features, as described below:

### Primary and secondary items

The item declarations on the first line of the input can contain a single pipe (`|`) that
separates primary items from secondary items. Primary items must be present in any solution
found by a solver but secondary items do not need to be present in a solution.

#### Example:

When `xc` runs on:

```
a b | c
a
b
a b c
```

it produces the output:

```
[src/xc.cc:256] Solution:
  1: a
  2: b

[src/xc.cc:256] Solution:
  3: a b c
```

_Supported by: xc, xcc, mcc_

### Colors

Options may specify a "color" for any secondary items. Colored secondary
items may be selected an unlimited number of times as long as the colors are
consistent.

Colors are specified by suffixing the item with a colon and the color.

#### Example:

When `xcc` runs on:

```
a b | c d
a c:RED d
b c:RED
b c:BLUE
a b c:BLUE
```

it produces the output:

```
[src/xcc.cc:318] Solution:
  1: a c:RED d
  2: b c:RED

[src/xcc.cc:318] Solution:
  4: a b c:BLUE
```

_Supported by: xcc, mcc_

### Multiplicities

Primary items can be annotated with upper and lower bounds on the number of times
they're used.

Multiplicities are defined by a pair of colon-delimited numbers within brackets after the
item declaration.

#### Example:

When `mcc` runs on:

```
A[0:1] B[1:2] C[2:3] D
A B
B C
A C
A B D
A C D
B C D
```

it produces the output:

```
[src/mcc.cc:456] Solution:
  5: A C D
  2: B C

[src/mcc.cc:456] Solution:
  6: B C D
  2: B C
  3: A C

[src/mcc.cc:456] Solution:
  6: B C D
  2: B C

[src/mcc.cc:456] Solution:
  6: B C D
  3: A C
```

_Supported by: mcc_

### Sharp prefixes

Items prefixed with a `#` can be treated specially by solvers, either by focusing
the search towards or away from these items. This can dramatically speed up the
solvers in some situations.

To enable this behavior, pass `-pprefer_sharp` or `-pprefer_unsharp` on the
command line to any of the solvers.

_Supported by: xc, xcc, mcc_