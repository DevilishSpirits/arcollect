#!/usr/bin/python3
from xml.sax.saxutils import quoteattr
import polib
import re
import sys

msgextract = sys.argv[1]
pofile     = polib.pofile(sys.argv[2])
wxl        = open(sys.argv[3],'w',encoding=pofile.encoding)

print(f"<?xml version='1.0' encoding='{pofile.encoding}'?>", file = wxl)
print('<?include $(config_wxi) ?>', file = wxl)
print('<WixLocalization xmlns="http://wixtoolset.org/schemas/v4/wxl" Codepage=',quoteattr(pofile.encoding),' Culture=',quoteattr(pofile.metadata["Language"].replace('_','-')),'>', file = wxl, sep='')

# Filter-out untranslated entries in non C locale
if msgextract != 'msgid':
	pofile = filter(polib.POEntry.translated,pofile)

for msg in pofile:
	print('\t<String Id="',msg.msgctxt,'" Value=',quoteattr(msg.__getattribute__(msgextract).replace('\n','\r\n')),'/>', file = wxl, sep='')
print('</WixLocalization>', file = wxl)
