Partridge Puzzle
================

Robert T. Wainright's [Partridge Puzzle](https://www.mathpuzzle.com/partridge.html) of order n is: pack 1 1-by-1 square,
2 2-by-2 squares, ..., n n-by-n squares into a larger square of size N-by-N, where N is the sum of the first
n cubes.

The sum of the areas of the smaller squares is always equal to the area of the larger square (N-by-N) because
of a basic identity about cubes: the sum of the first n cubes is always equal to the square of the sum of the
first n integers.

But just because the area sums work out doesn't mean that you can actually tile the larger square correctly.
In fact, there are no solutions for n < 8. But n = 8 has several nice packings (n = 12 does as well, and maybe
larger values of n?) and you can find them all with the script in this directory.

Run `./solve-partridge.sh 8` to show all of the solutions for n = 8. You'll have to wait a few minutes for
the first one to show up. You should see output like this at the terminal:

```
┌───┬───┬─────────┬─────────────┬─────────────┬─────────────┬───────────┐
│  2│  2│         │             │             │             │           │
├───┴───┤         │             │             │             │           │
│       │         │             │             │             │           │
│       │        5│             │             │             │           │
│      4├─────────┤             │             │             │          6│
├───────┤         │            7│            7│            7├───────────┤
│       │         ├─────────────┴─┬───────────┴───┬─────────┤           │
│       │         │               │               │         │           │
│      4│        5│               │               │         │           │
├─────┬─┴─────────┤               │               │         │           │
│     │           │               │               │        5│          6│
│    3│           │               │               ├─────┬───┴───────────┤
├─────┤           │               │               │     │               │
│     │           │              8│              8│    3│               │
│    3│          6├─┬───────────┬─┴─────┬─────────┴─────┤               │
├─────┴───┬───────┴─┤           │       │               │               │
│         │         │           │       │               │               │
│         │         │           │      4│               │               │
│         │         │           ├───────┤               │              8│
│        5│        5│          6│       │               ├───────────────┤
├─────────┴─────┬───┴───────────┤       │               │               │
│               │               │      4│              8│               │
│               │               ├───────┴───┬───────────┤               │
│               │               │           │           │               │
│               │               │           │           │               │
│               │               │           │           │               │
│               │               │           │           │              8│
│              8│              8│          6│          6├───────────────┤
├─────────────┬─┴───────────┬───┴─────────┬─┴───────────┤               │
│             │             │             │             │               │
│             │             │             │             │               │
│             │             │             │             │               │
│             │             │             │             │               │
│             │             │             │             │               │
│            7│            7│            7│            7│              8│
└─────────────┴─────────────┴─────────────┴─────────────┴───────────────┘
```