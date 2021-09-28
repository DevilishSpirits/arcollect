#!/usr/bin/python3
# Usage: webext-adder-test.py test_set...
from struct import pack
import sys

for test_set_path in sys.argv[1:]:
	json_data = open(test_set_path,'rb').read()
	sys.stdout.buffer.write(pack('I',len(json_data)))
	sys.stdout.buffer.write(json_data)
