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

giflib = static_library('giflib', sources, dependencies: dep_m)

dep_giflib = declare_dependency(link_with: giflib, dependencies: dep_m, include_directories: include_directories('.'))
