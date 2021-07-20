project('bzip2', 'c', version: '1.0.8')
incdirs = include_directories('.')
srcs = [
	'blocksort.c',
	'huffman.c',
	'crctable.c',
	'randtable.c',
	'compress.c',
	'decompress.c',
	'bzlib.c',
]
bzip2 = static_library('bzip2', srcs, include_directories: incdirs)
bzip2_dep = declare_dependency(link_with: bzip2, include_directories: incdirs)
