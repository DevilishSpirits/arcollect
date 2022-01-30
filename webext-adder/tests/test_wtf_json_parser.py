#!/usr/bin/python3
import json
from subprocess import Popen,PIPE
import sys
test_wtf_json_parser = sys.argv[1]
print('1..'+str(len(sys.argv)-2))
test_num = 1
for filename in sys.argv[2:]:
	data = open(filename,'r', encoding="utf-8").read()
	json_file = json.loads(data)
	parsed_json = Popen(test_wtf_json_parser,stdin=PIPE,stdout=PIPE).communicate(input=data.encode('utf-8'))[0].decode('utf-8')
	try:
		json_parsed = json.loads(parsed_json)
	except:
		json_parsed = None
	if json_file != json_parsed:
		print('not', end = ' ')
		print('\n# Parser sent this', parsed_json, file = sys.stderr)
		print('\n# I understood that', json.dumps(json_parsed), file = sys.stderr)
		print('\n# I expected that', json.dumps(json_file), file = sys.stderr)
	print('ok',test_num,'-',filename)
	test_num += 1
