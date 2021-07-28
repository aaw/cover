SCRIPT="$(realpath $0)"
SCRIPTDIR="$(dirname $SCRIPT)"
BASEDIR="$(dirname $SCRIPTDIR)"
cd "$BASEDIR"
make 1>/dev/null
./bin/xc <(./translate/encode-sudoku.py "$1") | ./translate/decode-sudoku.py "$1"
