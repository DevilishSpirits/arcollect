project('mingw-std-threads','cpp', version: '1.0.0')

headers = [
	'future',
	'invoke',
	'mutex',
	'shared_mutex',
	'thread',
]

foreach header: headers
	configure_file(input: 'mingw.'+header+'.h', output: header, copy: true)
endforeach

std_threaddep = declare_dependency(include_directories: include_directories('.'))
