#!/bin/bash

# Generate wordcross placements. Usage: create-wordcross.sh wordfile numrows numcols
# generates wordcross tilings that are numrows x numcols and contain all of
# the words in wordfile. Words in wordfile should be one per line.

SCRIPT="$(realpath $0)"
SCRIPTDIR="$(dirname $SCRIPT)"
EXAMPLEDIR="$(dirname $SCRIPTDIR)"
BASEDIR="$(dirname $EXAMPLEDIR)"
cd "$BASEDIR"
make 1>/dev/null
./bin/xcc <(./examples/wordcross/encode-wordcross.py "$1" "$2" "$3") | ./examples/wordcross/decode-wordcross.py "$2" "$3"
