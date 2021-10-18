Sudoku
======

[Sudoku](https://en.wikipedia.org/wiki/Sudoku) is a popular puzzle game. This directory contains
a fast Sudoku solver that translates the puzzle into an exact set cover problem.

The input to the solver is an 81-character string that contains the contents of the board you'd
like to solve in row-major order with a '.' in the place of each unknown cell. The output is the
same but with no unknown cells.

For example, the following puzzle:

![Puzzle image by Tim Stellmach, CC0, https://commons.wikimedia.org/w/index.php?curid=57831926](./example-puzzle.svg)

Is encoded as "53..7....6..195....98....6.8...6...34..8.3..17...2...6.6....28....419..5....8..79".

You can pass this to the solver by running:

```
$./solve-sudoku.sh "53..7....6..195....98....6.8...6...34..8.3..17...2...6.6....28....419..5....8..79"
```

Which should print the solution

```
534678912672195348198342567859761423426853791713924856961537284287419635345286179
```

The script `solve-all-hard-sudokus.sh` exercises the solver by downloading the list of hard
puzzles at http://magictour.free.fr/top95 and solving them all.
