#!/usr/bin/python3
import re
import sys

po_in = open(sys.argv[1],'r')
out = open(sys.argv[2],'w')
print('\t\t\t// Generated by po-hpp-gen.py', file = out)

messages = []
for line in po_in.readlines():
	line = line.strip().split(' ',1)
	if len(line) > 1:
		if line[0] == 'msgctxt':
			print('\t\t\tstd::string_view',line[1][1:-1]+';', file = out)
			messages.append(line[1])
print('\t\t\tconstexpr static std::array<std::string_view,'+str(len(messages))+'> po_strings = {',', '.join(messages),'};', file = out)
