project('OpenImageIO', 'cpp', 'c', version: '2.3.11.0')
## Copyright 2008-present Contributors to the OpenImageIO project.
## SPDX-License-Identifier: BSD-3-Clause
## https://github.com/OpenImageIO/oiio/blob/master/LICENSE.md

cpp = meson.get_compiler('cpp')

#set (PROJ_NAME OIIO)    # short name, caps

#option (EMBEDPLUGINS "Embed format plugins in libOpenImageIO" ON)
#set (PLUGIN_SEARCH_PATH "" CACHE STRING "Default plugin search path")
#file (TO_NATIVE_PATH "${PLUGIN_SEARCH_PATH}" PLUGIN_SEARCH_PATH_NATIVE)

#option (OIIO_THREAD_ALLOW_DCLP "OIIO threads may use DCLP for speed" ON)
#if (NOT OIIO_THREAD_ALLOW_DCLP)
#    add_definitions ("-DOIIO_THREAD_ALLOW_DCLP=0")
#endif ()

#set (TEX_BATCH_SIZE "" CACHE STRING "Force TextureSystem SIMD batch size (e.g. 16)")
#if (TEX_BATCH_SIZE)
#    add_definitions ("-DOIIO_TEXTURE_SIMD_BATCH_WIDTH=${TEX_BATCH_SIZE}")
#endif ()

#if (NUKE_FOUND)
#    add_subdirectory (src/nuke/txReader)
#    add_subdirectory (src/nuke/txWriter)
#endif ()

add_project_arguments('-DOIIO_STATIC_DEFINE', language: 'cpp')

dep_fmt   = dependency('fmt')
dep_imath = dependency('Imath')
dep_tiff  = dependency('libtiff-4')
dep_robin_map = dependency('robin-map')
dep_boost = dependency('boost', version: '>=1.53', modules: ['filesystem', 'thread'], default_options: [
	'BOOST_FILESYSTEM_NO_DEPRECATED=true',
	'BOOST_FILESYSTEM_DISABLE_BCRYPT=true',
])
oiio_deps = [
	dep_fmt,
	dep_imath,
	dep_tiff,
	dep_robin_map,
	dep_boost,
]

subdir('src')

dep_oiio = declare_dependency(link_with: OpenImageIO, include_directories: include_directories('src/include'), dependencies: oiio_deps, compile_args: '-DOIIO_STATIC_DEFINE')
