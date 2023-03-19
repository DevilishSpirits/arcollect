#!/usr/bin/python3
import json
from subprocess import Popen, PIPE
import os
import sys

class WebextAdder:
	prog_path = os.environ['ARCOLLECT_WEBEXT_ADDER_PATH']
	encoding = 'utf-8'
	def __init__(self, data_home: str, stdin):
		"Start the WebextAdder"
		"data_home: The location of the user collection (ARCOLLECT_DATA_HOME)"
		"stdin: The file to use for stdin (see Popen stdin value for extra informations)."
		
		if not isinstance(data_home,str):
			raise TypeError("data_home is not a string")
		env = dict(os.environ) # Clone parent environment
		env.update(
			ARCOLLECT_DATA_HOME = data_home,
		)
		self.data_home = data_home
		self.stdin = stdin
		self.process = Popen(['arcollect-webext-adder'],executable=self.prog_path,stdin=stdin,stdout=PIPE,stderr=None,shell=False,env=env,text=False)
	
	def command_line(self):
		"Return the command-line to reproduce this test"
		if hasattr(self.stdin,'name'):
			return f"ARCOLLECT_DATA_HOME='{self.data_home}' '{self.prog_path}' < '{self.stdin.name}'"
		else:
			return f"ARCOLLECT_DATA_HOME='{self.data_home}' '{self.prog_path}' # input is {self.stdin}"
	
	def read1(self):
		"Return a request result"
		return json.loads(self.process.stdout.read(int.from_bytes(self.process.stdout.read(4),signed=False,byteorder=sys.byteorder)).decode(self.encoding))
	
	def close(self):
		"Stop the underlying process"
		"This function may deadlock if the underlying process is stuck at writing to stdout"
		self.process.wait()
	
	def __repr__(self):
		if hasattr(self.stdin,'name'):
			input_repr = self.stdin.name
		else:
			input_repr = repr(self.stdin)
		return f'<WebextAdder pid={self.process.pid} input="{input_repr}">'
	def __enter__(self):
		return self
	def __exit__(self,exc_type,exc_value,traceback):
		self.close()
