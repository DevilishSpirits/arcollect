#!/usr/bin/python3
import json
from subprocess import Popen,PIPE
import sys
test_wtf_json_parser = sys.argv[1]
print('TAP version 13\n1..'+str(len(sys.argv)-2))
test_num = 1
for filename in sys.argv[2:]:
	data = open(filename,'r', encoding="utf-8").read()
	json_file = json.loads(data)
	try:
		parsed_json = Popen(test_wtf_json_parser,stdin=PIPE,stdout=PIPE).communicate(input=data.encode('utf-8'))[0]
		parsed_json = parsed_json.decode('utf-8')
		json_parsed = json.loads(parsed_json)
	except Exception as e:
		print('not', end = ' ')
		print('\n# Exception while parsing',filename,'result:', e, file = sys.stderr)
		print('\n# Parser sent this', parsed_json, file = sys.stderr)
	else:
		if json_file != json_parsed:
			print('not', end = ' ')
			print('\n# Parser sent this', parsed_json, file = sys.stderr)
			print('\n# I understood that', json.dumps(json_parsed), file = sys.stderr)
			print('\n# I expected that', json.dumps(json_file), file = sys.stderr)
	print('ok',test_num,'-',filename)
	test_num += 1
