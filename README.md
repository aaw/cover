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

Building
--------

Advanced features
-----------------
