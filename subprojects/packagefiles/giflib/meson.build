# GIFLIB meson.build for embeding
#
# Copyright (C) 2021 DevilishSpirits (aka D-Spirits or Luc B.)
#
# This file is distributed without any warranty and is not endorsed by the
# "GIFLIB" project <http://giflib.sourceforge.net/>.
#
# This file is based on the upstream Makefile and use the upstream license with
# a copy provided below.
#
# This license notice only apply to this file. Arcollect is published under the
# GPL-3.0 license with some part being explicitely shared under another license.
#
# Copy of the GIFLIB license :
# The GIFLIB distribution is Copyright (c) 1997  Eric S. Raymond
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
project('giflib', 'c', version: '5.2.1', default_options: [
	'c_std=gnu99',
])

cc = meson.get_compiler('c')
add_project_arguments(cc.get_supported_arguments('-Wno-format-truncation'), language: 'c')

if cc.has_function('reallocarray', prefix: '#include <stdlib.h>')
	add_project_arguments('-DHAVE_REALLOCARRAY', language: 'c')
endif

sources = [
	'dgif_lib.c',
	'egif_lib.c',
	'gifalloc.c',
	'gif_err.c',
	'gif_font.c',
	'gif_hash.c',
	'openbsd-reallocarray.c',
]

dep_m = cc.find_library('m', required: false)

giflib = static_library('giflib', sources, dependencies: dep_m, build_by_default: false)

dep_giflib = declare_dependency(link_with: giflib, dependencies: dep_m, include_directories: include_directories('.'))
