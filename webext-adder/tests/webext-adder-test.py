#!/usr/bin/python3
# Usage: webext-adder-test.py $XDG_DATA_HOME test_set...
from importlib.util import module_from_spec, spec_from_file_location
from os import path
from subprocess import Popen,PIPE
import db_check
import importlib
import json
import os
import requests
import shutil
import struct
import subprocess
import sys

# Open test sets
test_sets  = []
test_count = 0
for test_set_path in sys.argv[2:]:
	test_set = json.load(open(test_set_path,'r'))
	test_count += db_check.test_count(test_set)
	test_sets.append(test_set)

# Set XDG_DATA_HOME
os.environ['XDG_DATA_HOME'] = os.path.abspath(sys.argv[1])
shutil.rmtree(os.environ['XDG_DATA_HOME']+'/arcollect',ignore_errors=True)
os.makedirs(os.environ['XDG_DATA_HOME'],exist_ok=True)


# Number of tests ran at...
test_count  = 1                                 # Feeding check
for test_set in test_sets:
	test_count += db_check.test_count(test_set)     # DB checks
print('1..'+str(test_count))

# Feed webext-adder
print('# Feeding webext-adder')
webext_adder_input = bytes()
for test_set in test_sets:
	json_data = json.dumps(test_set).encode('utf-8')
	webext_adder_input += struct.pack('I',len(json_data))
	webext_adder_input += json_data

webext_adder_prog = os.environ['ARCOLLECT_WEBEXT_ADDER_PATH']
stdout = subprocess.Popen(webext_adder_prog,stdin=PIPE,stdout=PIPE).communicate(input=webext_adder_input)[0]
# FIXME Don't crash if no valid results are returned
stdout = json.loads(stdout[4:].decode('utf-8'))
if stdout['success']:
	print('ok',1,'- Web-ext adder')
else:
	print('not ok',1,'- Web-ext adder #',stdout['reason'])
	print('Bail Out! Addition failed')

print('# Starting database checks')
db = db_check.open_db()
for test_set in test_sets:
	db_check.check_db(1,db,test_set)
