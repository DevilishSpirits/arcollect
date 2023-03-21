project('OpenImageIO', 'cpp', 'c', version: '2.4.4.2')
## Copyright 2008-present Contributors to the OpenImageIO project.
## SPDX-License-Identifier: BSD-3-Clause
## https://github.com/OpenImageIO/oiio/blob/master/LICENSE.md

cpp = meson.get_compiler('cpp')

add_project_arguments('-DOIIO_STATIC_DEFINE', language: 'cpp')

compile_args_priv = [
	'-DUSE_STD_FILESYSTEM',
]

boost_modules = [
	'thread',
]

dep_fmt   = dependency('fmt')
dep_imath = dependency('Imath')
dep_imath_version = dep_imath.version().split('.')
dep_tiff  = dependency('libtiff-4')
dep_robin_map = dependency('tsl-robin-map', modules :'tsl::robin_map')
dep_boost = dependency('boost', version: '>=1.53', modules: boost_modules, default_options: [
])
dep_zlib  = dependency('zlib')
oiio_deps = [
	dep_fmt,
	dep_imath,
	dep_tiff,
	dep_robin_map,
	dep_boost,
	dep_zlib,
]

subdir('src')

dep_oiio = declare_dependency(link_with: OpenImageIO, include_directories: include_directories('src/include'), dependencies: oiio_deps, compile_args: '-DOIIO_STATIC_DEFINE')
