#!/usr/bin/python3
# Usage: test_payloads.py generate payload-output
# Usage: test_payloads.py run_test payload-input ARCOLLECT_DATA_HOME # /!\ WILL NUKE THE ARCOLLECT_DATA_HOME!!!

dummy_data = 'data:text/plain;base64,QXJjb2xsZWN0'

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
	# Ensure we fails when an a_b_links map to an non existent item
	'Fails on linking to non existent entry': [{
		'test.name': 'Init db',
		'test.success': True,
		'accounts': [{'id': "acc1",'url':'https://example.com','icon':dummy_data}],
		'artworks': [{'source': "art1",'data':dummy_data}],
		'comics': [{'id': "com1",'title':'Comic'}],
		'tagss': [{'id': "tag1",}],
	},{
		'test.name': 'art_acc_links test bad art',
		'test.success': False,
		'art_acc_links':[{"artwork": "art bad","account": 'acc1'}],
	},{
		'test.name': 'art_acc_links test bad acc',
		'test.success': False,
		'art_acc_links':[{"artwork": "art1","account": 'acc bad'}],
	},{
		'test.name': 'art_tag_links test bad art',
		'test.success': False,
		'art_tag_links':[{"artwork": "art bad","tag": 'tag1'}],
	},{
		'test.name': 'art_tag_links test bad tag',
		'test.success': False,
		'art_tag_links':[{"artwork": "art1","tag": 'acc bad'}],
	},{
		'test.name': 'com_acc_links test bad art',
		'test.success': False,
		'com_acc_links':[{"comic": "art bad","account": 'acc1'}],
	},{
		'test.name': 'com_acc_links test bad acc',
		'test.success': False,
		'com_acc_links':[{"comic": "com1","account": 'acc bad'}],
	},{
		'test.name': 'com_tag_links test bad art',
		'test.success': False,
		'com_tag_links':[{"comic": "art bad","tag": 'tag1'}],
	},{
		'test.name': 'com_tag_links test bad tag',
		'test.success': False,
		'com_tag_links':[{"comic": "com1","tag": 'acc bad'}],
	}],
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
