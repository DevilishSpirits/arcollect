#!/bin/bash
# Usage : @CURRENT_SOURCE_DIR@ @CURRENT_BUILD_DIR@ zip-executable output inputs...
OUTPUT="$PWD/$4" # TODO Works with absolute paths
OLDPWD="$PWD"
CURRENT_SOURCE_DIR="$1/"
CURRENT_BUILD_DIR="$2/"
PROG_ZIP="$3"
shift 3
rm -f "$OUTPUT" # Remove old ZIP
cd "$CURRENT_BUILD_DIR"
$PROG_ZIP "$OUTPUT" $(for file in $*; do test -f "$(basename "$file")" && echo "$(basename "$file")"; done) || exit "$?"
cd "$OLDPWD"
cd "$CURRENT_SOURCE_DIR"
exec $PROG_ZIP "$OUTPUT" $(for file in $*; do test -f "${file:${#CURRENT_SOURCE_DIR}}" && echo "${file:${#CURRENT_SOURCE_DIR}}"; done)
