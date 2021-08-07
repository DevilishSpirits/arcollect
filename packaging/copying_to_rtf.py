#!/usr/bin/python3
import re
import sys
sys.stdin.reconfigure(encoding='windows-1252')
sys.stdout.reconfigure(encoding='windows-1252',newline='\r\n')
print('{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl{\\f0\\fnil\\fcharset0 Courier New;}}')
print('{\\colortbl ;\\red0\\green0\\blue255;}')
print('\\viewkind4\\uc1\\pard\\sl240\\slmult1\\lang1033\\f0\\fs14 ', end='')

for line in sys.stdin:
	line = line.rstrip("\r\n")
	for hyperlink in frozenset(re.findall(r'\<https[:/\.a-z0-9-]+\>',line)):
		hyperlink = hyperlink[1:-1]
		line = line.replace('<'+hyperlink+'>','<{\\field{\\*\\fldinst{HYPERLINK "'+hyperlink+'"}}{\\fldrslt{\\ul\\cf1 '+hyperlink+'}}}\\f0\\fs14 >')
	print(line+'\\par')
print('}\r\n\\00')
