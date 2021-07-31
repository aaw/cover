#!/bin/bash

# Solves all 95 hard sudoku puzzles from a list on magictour.free.fr.

SCRIPT="$(realpath $0)"
SCRIPTDIR="$(dirname $SCRIPT)"
for puzzle in $(wget -qO- http://magictour.free.fr/top95); do
    [ "$(expr length $puzzle)" -ne 81 ] && continue
    echo "Solving [${puzzle}]"
    "${SCRIPTDIR}/solve-sudoku.sh" "$puzzle"
done
