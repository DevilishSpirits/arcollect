#!/usr/bin/python3
# Usage: webextension-browser-test.py browser_name platform_name test_set
import importlib
from importlib.util import module_from_spec, spec_from_file_location
import json
import os
from os import path
import requests
import shutil
import sqlite3
import sys
import WebDriver
# import db_check
db_check_path = path.dirname(__file__)+'/../../webext-adder/tests/db_check.py'
db_check_spec   = spec_from_file_location('db_check',db_check_path)
db_check        = module_from_spec(db_check_spec)
db_check_spec.loader.exec_module(db_check)

browser_name  = sys.argv[1]
platform_name = sys.argv[2]
test_set_path = sys.argv[3]
test_set_name = path.basename(test_set_path)

browser_module  = importlib.import_module(browser_name , package='.')
platform_module = importlib.import_module(platform_name, package='.')
test_set        = json.load(open(test_set_path,'r'))
test_num        = browser_module.Browser.test_num()

# Create a custom XDG_DATA_HOME
if 'ARCOLLECT_TEST_DATA_HOME' not in os.environ:
	os.environ['ARCOLLECT_TEST_DATA_HOME'] = '.'
os.environ['XDG_DATA_HOME'] = os.path.abspath(\
os.environ['ARCOLLECT_TEST_DATA_HOME']+\
'/'+browser_name[8:]+'_'+platform_name[9:]+'_'+test_set_name)
shutil.rmtree(os.environ['XDG_DATA_HOME'],ignore_errors=True)
os.makedirs(os.environ['XDG_DATA_HOME'],exist_ok=True)
print("# XDG_DATA_HOME set to",os.environ['XDG_DATA_HOME'])

## Tables on which to perform line count checks
# 
# This tuple list tables on which a line count check must be made. To pass the
# line count in the database must be the same as in the test_suite array.
db_line_count_checks = db_check.db_line_count_checks

# Number of tests ran at...
test_count  = browser_module.Browser.test_num() # Browser() init
test_count += len(db_check.db_line_count_checks)    # Line count checks
test_count += len(test_set['artworks'])         # Browser interaction
test_count += db_check.test_count(test_set)     # DB checks
print('1..'+str(test_count))

try:
	browser   = browser_module.Browser()
	webdriver = browser.webdriver
	platform  = platform_module.Platform(webdriver)
	
	# Browser interaction tests
	for artwork in test_set['artworks']:
		print('# Trying',artwork['source'])
		test_num += 1
		if webdriver.navigateTo(artwork['source']).status_code == requests.codes.ok:
			# Perform interaction
			if platform():
				print('ok',test_num,'- Saving artwork',artwork['source'])
			else:
				print('not ok',test_num,'- Saving artwork',artwork['source'])
		else:
			print('not ok',test_num,'- Saving artwork',artwork['source'],'# Browsing to page failed')
	# Free browser
	print('# Browser interactions test done. Closing browser')
	platform  = None
	webdriver = None
	browser   = None
	print('# Starting database checks')
	db = db_check.open_db()
	db_check.linecount_checks(test_num,db,[test_set])
	test_num += len(db_check.db_line_count_checks)
	db_check.check_db(test_num,db,test_set)
except:
	raise
finally:
	# Destroy things
	platform  = None
	webdriver = None
	browser   = None
