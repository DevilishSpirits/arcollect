# libpng meson.build for embeding
#
# Copyright (C) 2021 DevilishSpirits (aka D-Spirits or Luc B.)
# libpng's CMakeLists.txt contributor list :
# Copyright (C) 2018 Cosmin Truta
# Copyright (C) 2007,2009-2018 Glenn Randers-Pehrson
# Written by Christian Ehrlicher, 2007
# Revised by Roger Lowman, 2009-2010
# Revised by Clifford Yapp, 2011-2012,2017
# Revised by Roger Leigh, 2016
# Revised by Andreas Franek, 2016
# Revised by Sam Serrels, 2017
# Revised by Vadim Barkov, 2017
# Revised by Vicky Pfau, 2018
# Revised by Cameron Cawley, 2018
# Revised by Cosmin Truta, 2018
# Revised by Kyle Bentley, 2018
#
# This file is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h of your libpng distribution.
#
# This project is not endorsed by the
# "libpng" project <http://www.libpng.org/pub/png/libpng.html>.
#
# This file is based on the upstream CMakeLists.txt and use the same license.
#
# This license notice only apply to this file. Arcollect is published under the
# GPL-3.0 license with some part being explicitely shared under another license.
project('libpng', 'c', version: '1.6.37')
cc = meson.get_compiler('c')
zlib_dep = dependency('zlib', method: 'system')
m_dep = cc.find_library('m', required: false)
configure_file(input: 'scripts/pnglibconf.h.prebuilt', output: 'pnglibconf.h', copy: true)
libpng_public_hdrs = [
	'png.h',
	'pngconf.h',
]
libpng_private_hdrs = [
	'pngpriv.h',
	'pngdebug.h',
	'pnginfo.h',
	'pngstruct.h',
]
libpng_sources = [
	libpng_public_hdrs,
	libpng_private_hdrs,
	'png.c',
	'pngerror.c',
	'pngget.c',
	'pngmem.c',
	'pngpread.c',
	'pngread.c',
	'pngrio.c',
	'pngrtran.c',
	'pngrutil.c',
	'pngset.c',
	'pngtrans.c',
	'pngwio.c',
	'pngwrite.c',
	'pngwtran.c',
	'pngwutil.c',
	#${libpng_arm_sources}
	#${libpng_intel_sources}
	#${libpng_mips_sources}
	#${libpng_powerpc_sources}
]
png_incs   = include_directories('.')
png_static = static_library('png_static', libpng_sources, dependencies: [zlib_dep, m_dep], include_directories: png_incs)

libpng_dep = declare_dependency(link_with: png_static, dependencies: [zlib_dep, m_dep], include_directories: png_incs)
png_dep    = libpng_dep
