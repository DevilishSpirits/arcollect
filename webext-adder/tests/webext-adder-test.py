#!/usr/bin/python3
# Usage: webext-adder-test.py $XDG_DATA_HOME test_payload test_set...
from importlib.util import module_from_spec, spec_from_file_location
from os import path
import db_check
import importlib
import json
import os
import requests
import shutil
from struct import unpack
import sys
from WebextAdder import WebextAdder

# Open test sets
test_sets  = []
test_count = 0
for test_set_path in sys.argv[3:]:
	test_set = json.load(open(test_set_path,'r'))
	test_count += db_check.test_count(test_set)
	test_sets.append(test_set)

# Set XDG_DATA_HOME
os.environ['XDG_DATA_HOME'] = os.path.abspath(sys.argv[1])
shutil.rmtree(os.environ['XDG_DATA_HOME']+'/arcollect',ignore_errors=True)
os.makedirs(os.environ['XDG_DATA_HOME'],exist_ok=True)


# Number of tests ran at...
test_count  = len(test_sets)                  # Feeding checks
for test_set in test_sets:
	test_count += db_check.test_count(test_set) # DB checks
print('TAP version 13\n1..'+str(test_count))

# Feed webext-adder
print('# Starting webext-adder')
webext_success = True
test_num = 0
with WebextAdder(os.environ['XDG_DATA_HOME']+'/arcollect',open(sys.argv[2],'rb')) as webext_adder:
	print(webext_adder.command_line(), file = sys.stderr)
	
	for test_set_path in sys.argv[3:]:
		test_num += 1
		# Read the JSON
		try:
			stdout_json = webext_adder.read1()
		except Exception as e:
		# FIXME Don't crash if no valid results are returned
			print(e,file = sys.stderr)
			stdout_json = {'success': False, 'reason': 'Exception when reading the JSON'}
		if stdout_json['success']:
			print('ok',test_num,'- Web-ext adder for',test_set_path)
		else:
			print('not ok',test_num,'- Web-ext adder for',test_set_path,'#',stdout_json['reason'])
			webext_success = False
	print('# Waiting for the webext-adder to shutdown')

if not webext_success:
	print('Bail out! Addition failed')
	sys.exit(1)

print('# Starting database checks')
db = db_check.open_db()
for test_set in test_sets:
	test_num = db_check.check_db(test_num,db,test_set)
