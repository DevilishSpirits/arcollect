# boost::filesystem meson.build for embeding
#
# Copyright 2021, DevilishSpirits (aka D-Spirits or Luc B.)
# Boost's CMakeLists.txt contributor list :
# Copyright 2019 Mike Dev
# Copyright 2020, 2021 Andrey Semashev
#
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE.build or copy at https://www.boost.org/LICENSE_1_0.txt
#
# This project is not endorsed by the
# "Boost" project <https://boost.org/>.
#
# This file is based on the upstream CMakeLists.txt and use the same license.
#
# This license notice only apply to this file. Arcollect is published under the
# GPL-3.0 license with some part being explicitely shared under another license.
BOOST_FILESYSTEM_ARGS = []
if get_option('BOOST_FILESYSTEM_NO_DEPRECATED')
	BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_NO_DEPRECATED'
endif
if get_option('BOOST_FILESYSTEM_DISABLE_SENDFILE')
	BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_DISABLE_SENDFILE'
endif
if get_option('BOOST_FILESYSTEM_DISABLE_COPY_FILE_RANGE')
	BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_DISABLE_COPY_FILE_RANGE'
endif
if get_option('BOOST_FILESYSTEM_DISABLE_GETRANDOM')
	BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_DISABLE_GETRANDOM'
endif
if get_option('BOOST_FILESYSTEM_DISABLE_ARC4RANDOM')
	BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_DISABLE_ARC4RANDOM'
endif

feature_tests = {
	'has_attribute_init_priority': 'HAS_INIT_PRIORITY',
	'has_stat_st_blksize': 'HAS_STAT_ST_BLKSIZE',
	'has_stat_st_mtim': 'HAS_STAT_ST_MTIM',
	'has_stat_st_mtimensec': 'HAS_STAT_ST_MTIMENSEC',
	'has_stat_st_birthtimensec': 'HAS_STAT_ST_BIRTHTIMENSEC',
	'has_stat_st_birthtimespec': 'HAS_STAT_ST_BIRTHTIMESPEC',
}

if get_option('BOOST_FILESYSTEM_DISABLE_STATX')
	BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_DISABLE_STATX'
else
	feature_tests += {'has_statx': 'HAS_STATX', 'has_statx_syscall': 'HAS_STATX_SYSCALL'}
endif

foreach test, DEFINE: feature_tests
	if cpp.compiles('config/'+test+'.cpp', name: test)
		BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_'+DEFINE
	endif
endforeach

# TODO. Also note that we are in C++17 for now

#if cpp.compiles('config/has_cxx20_atomic_ref.cpp', name: 'has_cxx20_atomic_ref')
	#BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_HAS_CXX20_ATOMIC_REF',
	#    target_link_libraries(boost_filesystem PRIVATE Boost::atomic)
#else
	BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_NO_CXX20_ATOMIC_REF'
#endif

# TODO We won't need it
BOOST_FILESYSTEM_ARGS += '-DBOOST_FILESYSTEM_DISABLE_BCRYPT'
#if(WIN32 AND NOT BOOST_FILESYSTEM_DISABLE_BCRYPT)
#    set(CMAKE_REQUIRED_LIBRARIES bcrypt)
#    check_cxx_source_compiles("#include <${CMAKE_CURRENT_SOURCE_DIR}/config/has_bcrypt.cpp>" BOOST_FILESYSTEM_HAS_BCRYPT)
#    unset(CMAKE_REQUIRED_LIBRARIES)
#endif()

BOOST_FILESYSTEM_SOURCES = files(
	'src/codecvt_error_category.cpp',
	'src/exception.cpp',
	'src/operations.cpp',
	'src/directory.cpp',
	'src/path.cpp',
	'src/path_traits.cpp',
	'src/portability.cpp',
	'src/unique_path.cpp',
	'src/utf8_codecvt_facet.cpp',
)
if (host_machine.system() == 'windows') or (host_machine.system() == 'cygwin')
	BOOST_FILESYSTEM_SOURCES += files('src/windows_file_codecvt.cpp')
	BOOST_FILESYSTEM_ARGS += [
		'-DBOOST_USE_WINDOWS_H',
		'-DWIN32_LEAN_AND_MEAN',
		'-DNOMINMAX',
	]
endif

boost_libs += static_library('boost-filesystem', BOOST_FILESYSTEM_SOURCES, include_directories: [boost_inc, include_directories('src')], cpp_args: BOOST_FILESYSTEM_ARGS, build_by_default: false)

#target_link_libraries(boost_filesystem
#    PUBLIC
#        Boost::assert
#        Boost::config
#        Boost::container_hash
#        Boost::core
#        Boost::detail
#        Boost::io
#        Boost::iterator
#        Boost::smart_ptr
#        Boost::system
#        Boost::type_traits

#    PRIVATE
#        Boost::predef
#)

#TODO
#if(WIN32)
#    if(BOOST_FILESYSTEM_HAS_BCRYPT)
#        target_compile_definitions(boost_filesystem PRIVATE BOOST_FILESYSTEM_HAS_BCRYPT)
#        target_link_libraries(boost_filesystem PRIVATE bcrypt)
#    else()
#        target_compile_definitions(boost_filesystem PRIVATE BOOST_FILESYSTEM_HAS_WINCRYPT)
#        if(NOT WINCE)
#            target_link_libraries(boost_filesystem PRIVATE advapi32)
#        else()
#            target_link_libraries(boost_filesystem PRIVATE coredll)
#        endif()
#    endif()

#    target_link_libraries(boost_filesystem
#        PRIVATE
#            Boost::winapi
#    )
#endif()
