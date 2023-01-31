# boost::thread meson.build for embeding
#
# Copyright 2021, DevilishSpirits (aka D-Spirits or Luc B.)
# Boost's CMakeLists.txt contributor list :
# Copyright 2020 Peter Dimov
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

BOOST_THREAD_ARGS = [
	'-DBOOST_THREAD_STATIC_LINK',
	'-DBOOST_THREAD_USE_LIB',
	'-DBOOST_THREAD_BUILD_LIB',
]

threadapi = get_option('BOOST_THREAD_THREADAPI')
if threadapi == 'auto'
	if (host_machine.system() == 'windows') or (host_machine.system() == 'cygwin')
		threadapi = 'win32'
	else
		threadapi = 'pthread'
	endif
endif

BOOST_THREAD_SOURCES = files(
)
if threadapi == 'win32'
	BOOST_THREAD_SOURCES += files(
		'src/win32/thread.cpp',
		'src/win32/tss_dll.cpp',
		'src/win32/tss_pe.cpp',
		'src/win32/thread_primitives.cpp',
	)
elif threadapi == 'pthread'
	BOOST_THREAD_SOURCES += files(
		'src/pthread/thread.cpp',
		'src/pthread/once.cpp',
	)
endif

boost_libs += static_library('boost-thread', BOOST_THREAD_SOURCES, include_directories: [boost_inc, include_directories('src')], build_by_default: false, cpp_args: BOOST_THREAD_ARGS)
