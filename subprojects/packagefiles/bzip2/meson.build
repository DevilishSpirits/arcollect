# bzip2 meson.build for embeding
#
# Written in 2021 by DevilishSpirits (aka D-Spirits or Luc B.)
#
# To the extent possible under law, the author(s) have dedicated all copyright
# and related and neighboring rights to this file to the public domain worldwide
# using the CC0 Public Domain Dedication.
# See <https://creativecommons.org/publicdomain/zero/1.0/>.
#
# This file is distributed without any warranty and is not endorsed by the
# "bzip2" project <https://www.sourceware.org/bzip2/>.
#
# This license notice only apply to this file. Arcollect is published under the
# GPL-3.0 license with some part being explicitely shared under another license.
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
bzip2 = static_library('bzip2', srcs, include_directories: incdirs, build_by_default: false)
bzip2_dep = declare_dependency(link_with: bzip2, include_directories: incdirs)
