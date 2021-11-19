#!/usr/bin/python3
# Usage : @CURRENT_BUILD_DIR@ messages.json output inputs...
from json import load as load_json, dumps as dump_json
from sys import argv
from zipfile import ZipFile
from pathlib import PurePath
build_dir  = PurePath(argv[1])
messages   = load_json(open(argv[2],'r'))
output     = argv[3]
inputs     = argv[4:]
with ZipFile(output,'w') as zipfile:
	# Static files
	for input_path in inputs:
		zipfile.write(build_dir/input_path,input_path)
	# Translations
	for lang in messages:
		zipfile.writestr('_locales/'+lang+'/messages.json',dump_json(messages[lang]))
