#!/bin/bash

SCRIPT="$(realpath $0)"
SCRIPTDIR="$(dirname $SCRIPT)"
EXAMPLEDIR="$(dirname $SCRIPTDIR)"
BASEDIR="$(dirname $EXAMPLEDIR)"
cd "$BASEDIR"
make 1>/dev/null
./bin/xc <(./examples/sudoku/encode-sudoku.py "$1") | ./examples/sudoku/decode-sudoku.py "$1"
