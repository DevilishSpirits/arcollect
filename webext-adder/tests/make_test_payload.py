#!/usr/bin/python3
# Usage: webext-adder-test.py test_set...
import json
import sys

encoding = 'utf-8'
"Encoding for the webext-adder JSON"

def payload2bytes(payload):
	"Generate a payload binary stream (without the leading u32 length)"
	return json.dumps(payload).encode(encoding)

def write_bytes_payload(output, *payloads):
	"Wrap a payload and write it to output"
	"output: an object with a write() method accepting bytse"
	"*payloads: Payloads bytes to write and wrap"
	for payload in payloads:
		output.write(len(payload).to_bytes(length=4,signed=False,byteorder=sys.byteorder))
		output.write(payload)

def write_payload(output, *payloads):
	"Write a payload to output"
	"output: an object with a write() method accepting bytes"
	"payloads: Payloads Python dict() to serialize"
	return write_bytes_payload(output,*map(payload2bytes,payloads))

if __name__ == '__main__':
	for test_set_path in sys.argv[1:]:
		write_bytes_payload(sys.stdout.buffer,open(test_set_path,'rb').read())
