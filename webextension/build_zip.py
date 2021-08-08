#!/usr/bin/python3
# Usage : @CURRENT_BUILD_DIR@ output inputs...
from sys import argv
from zipfile import ZipFile
from pathlib import PurePath
build_dir  = PurePath(argv[1])
output     = argv[2]
inputs     = argv[3:]
with ZipFile(output,'w') as zipfile:
	for input_path in inputs:
		zipfile.write(build_dir/input_path,input_path)
