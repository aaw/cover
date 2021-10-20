#!/bin/bash

# Generate word searches. Usage: create-wordsearch.sh wordfile numrows numcols
# generates word search puzzles that are numrows x numcols and contain all of
# the words in wordfile. Words in wordfile should be one per line.

FILEPATH="$(realpath $1)"
SCRIPT="$(realpath $0)"
SCRIPTDIR="$(dirname $SCRIPT)"
EXAMPLEDIR="$(dirname $SCRIPTDIR)"
BASEDIR="$(dirname $EXAMPLEDIR)"
cd "$BASEDIR"
make 1>/dev/null
./bin/xcc <(./examples/wordsearch/encode-wordsearch.py "$FILEPATH" "$2" "$3") | ./examples/wordsearch/decode-wordsearch.py "$2" "$3"
