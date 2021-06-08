#!/bin/sh
# Usage : zip-path build-dir unzip-executable web-ext-executable
TMPDIR="$2/web-ext-lint-test-workdir"
rm -rf "$TMPDIR" && mkdir "$TMPDIR" || exit "$?"
$3 "$1" -d "$TMPDIR" || exit "$?" # unzip-executable zip-path -d "$TMPDIR"
$4 lint -s "$TMPDIR" -a "$TMPDIR/web-ext-artifacts" --no-input --warnings-as-errors --no-config-discovery || exit "$?" # zip-path unzip-executable -d "$TMPDIR"
