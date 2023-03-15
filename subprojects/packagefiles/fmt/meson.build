# {fmt} meson.build for embeding
#
# Written in 2021 by DevilishSpirits (aka D-Spirits or Luc B.)
#
# To the extent possible under law, the author(s) have dedicated all copyright
# and related and neighboring rights to this file to the public domain worldwide
# using the CC0 Public Domain Dedication.
# See <https://creativecommons.org/publicdomain/zero/1.0/>.
#
# This file is distributed without any warranty and is not endorsed by the
# "{fmt}" project <https://fmt.dev/>.
#
# This license notice only apply to this file. Arcollect is published under the
# GPL-3.0 license with some part being explicitely shared under another license.
project('FMT', 'cpp')
incdirs = include_directories('include')
fmt = static_library('fmt', 'src/format.cc', 'src/os.cc', include_directories: incdirs, build_by_default: false)

fmt_ver = meson.get_compiler('cpp').get_define('FMT_VERSION', include_directories: incdirs, prefix: '#include <fmt/core.h>').to_int()
fmt_vermajor = fmt_ver/10000
fmt_ver = fmt_ver - fmt_vermajor*10000
fmt_verminor = fmt_ver/100
fmt_ver = fmt_ver - fmt_verminor*100
fmt_ver = fmt_vermajor.to_string()+'.'+fmt_verminor.to_string()+'.'+fmt_ver.to_string()

fmt_incdir = meson.current_source_dir()/'include'
fmt_dep = declare_dependency(link_with: fmt, include_directories: incdirs, version: fmt_ver)
