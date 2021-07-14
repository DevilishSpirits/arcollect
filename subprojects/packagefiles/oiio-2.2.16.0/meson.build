project('OpenImageIO', 'cpp', 'c', version: '2.2.16.0')
## Copyright 2008-present Contributors to the OpenImageIO project.
## SPDX-License-Identifier: BSD-3-Clause
## https://github.com/OpenImageIO/oiio/blob/master/LICENSE.md

cpp = meson.get_compiler('cpp')

#project (HOMEPAGE_URL "https://openimageio.org")
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

#if (NOT BUILD_OIIOUTIL_ONLY)
#    add_subdirectory (src/libOpenImageIO)
#endif ()

#add_subdirectory (src/include)
#add_subdirectory (src/fonts)

#if (NUKE_FOUND)
#    add_subdirectory (src/nuke/txReader)
#    add_subdirectory (src/nuke/txWriter)
#endif ()

## generate the config file from the template in the binary dir
#configure_package_config_file ("${PROJECT_SOURCE_DIR}/src/cmake/Config.cmake.in"
#        "${OIIO_PROJECT_CONFIG}"
#        INSTALL_DESTINATION "${OIIO_CONFIG_INSTALL_DIR}")

## generate the config file from the template in the binary dir
#install (FILES "${OIIO_PROJECT_CONFIG}" "${OIIO_VERSION_CONFIG}"
#        DESTINATION "${OIIO_CONFIG_INSTALL_DIR}")

add_project_arguments('-DOIIO_STATIC_DEFINE', language: 'cpp')

dep_fmt   = dependency('fmt')
dep_imath = dependency('Imath')
dep_tiff  = dependency('libtiff-4')
robin_map_dep = dependency('robin-map')
subdir('src')

boost_fs = cpp.find_library('boost_filesystem', required: false)
if not boost_fs.found()
	boost_fs = cpp.find_library('boost_filesystem-mt', required: false)
endif
boost_thread = cpp.find_library('boost_thread', required: false)
if not boost_thread.found()
	boost_thread = cpp.find_library('boost_thread-mt', required: false)
endif

oiio_deps = [
	dep_fmt,
	dep_imath,
	dep_tiff,
	boost_fs,
	boost_thread,
	cpp.find_library('dl', required: false),
	cpp.find_library('Bcrypt', required: false), # FIXME What boost::filesystem does with BCrypt ???
]

dep_oiio = declare_dependency(link_with: OpenImageIO, include_directories: include_directories('src/include'), dependencies: oiio_deps, compile_args: '-DOIIO_STATIC_DEFINE')
