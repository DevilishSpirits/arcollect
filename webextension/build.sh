#!/bin/sh
# Usage : cd-directory zip-executable output inputs...
OUTPUT="$PWD/$3"
CDDIR="$1/"
cd "$CDDIR"
PROG_ZIP="$2"
shift 3
rm -f "$OUTPUT" # Remove old ZIP
exec $PROG_ZIP "$OUTPUT" $(for file in $*; do echo ${file:${#CDDIR}}; done)
