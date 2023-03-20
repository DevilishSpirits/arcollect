#!/usr/bin/python
# Usage: test_payloads.py generate payload-output
# Usage: test_payloads.py run_test payload-input ARCOLLECT_DATA_HOME # /!\ WILL NUKE THE ARCOLLECT_DATA_HOME!!!
test_set = {
	# Example test
	'Example': [{ # 
		'test.name': 'Success', # Step title used in logs
		'test.success': True, # Weather the webext-adder should succeed or not
		# Put the payload below
		# Note: 'platform' is automatically set to the test name
		'artworks': [],
	},{ # Another test
		'test.name': 'Failure', # Step title used in logs
		'test.success': False, # Weather the webext-adder should succeed or not
		'artworks': {}, # This will fails
	}],
	# Ensure we reject non https:// URLs
	'Reject bad scheme': list(map(lambda scheme: {
		'test.name': scheme,
		'test.success': False,
		'artworks': [{'source':'dummy','data':scheme+'://example.com'}],
	},[
		# List from the curl man-page
		'dict',
		'file',
		'ftp',
		'ftps',
		'gopher',
		'gophers',
		'http',
		'imap',
		'imaps',
		'ldap',
		'ldaps',
		'mqtt',
		'pop3',
		'pop3s',
		'rtmp',
		'rtmps',
		'rtsp',
		'scp',
		'sftp',
		'smb',
		'smbs',
		'smtp',
		'smtps',
		'telnet',
		'tftp',
		'ws',
		'wss',
	])),
}

sorted_test_set = sorted(test_set.items())

def generate(output):
	import itertools
	from make_test_payload import write_payload
	write_payload(output,*itertools.chain(*map(lambda test: list(map(lambda step: step.update(platform=test[0]) or step, test[1])),sorted_test_set)))

if __name__ == '__main__':
	import sys
	if sys.argv[1] == 'generate':
		with open(sys.argv[2],'wb') as f:
			generate(f)
	elif sys.argv[1] == 'run_test':
		from WebextAdder import WebextAdder
		import os
		import shutil
		ARCOLLECT_DATA_HOME = sys.argv[3]
		# Cleanup 
		shutil.rmtree(ARCOLLECT_DATA_HOME,ignore_errors=True)
		os.makedirs(ARCOLLECT_DATA_HOME,exist_ok=True)
		# Count the number of tests
		test_count = sum(map(len,test_set.values()))
		test_num = 0
		print(f'TAP version 13\n1..{test_count}')
		print("# Starting the webext-adder")
		with WebextAdder(ARCOLLECT_DATA_HOME,open(sys.argv[2],'rb')) as webext_adder:
			for suite, steps in sorted_test_set:
				for test in steps:
					res = webext_adder.read1()
					if res['success'] != test['test.success']:
						print('not ',end='')
					if res['success']:
						reason = 'Success'
					else:
						reason = res['reason']
					test_num += 1
					print('ok',test_num,'-',suite,':',test['test.name'],'#',reason)
