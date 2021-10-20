Word search
===========

[Word search puzzles](https://en.wikipedia.org/wiki/Word_search) are common teaching tools for vocabulary words.
Words are hidden in a rectangle of letters horizontally, vertically, and diagonally, both forwards and backwards,
and may overlap.

The solver in this directory takes a list of words in a newline-delimited file and rectangle dimensions and
generates all word search puzzles containing all words in the file. Underscores in the resulting puzzle can be
replaced by any letter.

The file `numbers.txt` in this directory has the numbers ONE through TWELVE. Running `create-wordsearch.sh numbers.txt 6 6` generates puzzles like:

```
EERHTN
VLEUWE
LXENOT
EIEVIF
WSEVEN
THGIEN
```

The file `names.txt` in this directory has the last names of some famous mathematicians. `create-wordsearch.sh names.txt 13 13` generates puzzles like:

```
WEIERSTRASSKM
FROBENIUSTYII
HADAMARDGILRN
BERTRANDLEVCK
CATALANMALEHO
HERMITEAITSHW
UICANTORSJTOS
RNL__L_KHEEFK
WEABELNOESRFI
ISGSEOEFRGN__
TNNNPRTFBOREL
ZE_PU_TUADNAL
HJPERRONILLEM
```
