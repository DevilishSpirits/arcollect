# boost::atomic meson.build for embeding
#
# Copyright 2021, DevilishSpirits (aka D-Spirits or Luc B.)
# Boost's CMakeLists.txt contributor list :
# Copyright 2018 Mike Dev
# Copyright 2019 Peter Dimov
# Copyright 2020-2021 Andrey Semashev
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

BOOST_ATOMIC_ARGS = [
	'-DBOOST_ATOMIC_STATIC_LINK',
]
BOOST_ATOMIC_SOURCES = files(
	'src/lock_pool.cpp',
)
if (host_machine.system() == 'windows')
	BOOST_ATOMIC_SOURCES += files('src/wait_on_address.cpp')
endif

#check_cxx_source_compiles("#include <${CMAKE_CURRENT_SOURCE_DIR}/../config/checks/architecture/x86.cpp>\nint main() {}" BOOST_ATOMIC_TARGET_X86)

#if (BOOST_ATOMIC_TARGET_X86)
#    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
#            set(boost_atomic_sse2_cflags "/arch:SSE2")
#            set(boost_atomic_sse41_cflags "/arch:SSE2")
#        endif()
#    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
#        if (WIN32)
#            set(boost_atomic_sse2_cflags "/QxSSE2")
#            set(boost_atomic_sse41_cflags "/QxSSE4.1")
#        else()
#            set(boost_atomic_sse2_cflags "-xSSE2")
#            set(boost_atomic_sse41_cflags "-xSSE4.1")
#        endif()
#    else()
#        set(boost_atomic_sse2_cflags "-msse -msse2")
#        set(boost_atomic_sse41_cflags "-msse -msse2 -msse3 -mssse3 -msse4.1")
#    endif()

#    set(CMAKE_REQUIRED_INCLUDES ${BOOST_LIBRARY_INCLUDES})
#    set(CMAKE_REQUIRED_FLAGS "${boost_atomic_sse2_cflags}")
#    check_cxx_source_compiles("#include <${CMAKE_CURRENT_SOURCE_DIR}/config/has_sse2.cpp>" BOOST_ATOMIC_COMPILER_HAS_SSE2)
#    unset(CMAKE_REQUIRED_FLAGS)
#    unset(CMAKE_REQUIRED_INCLUDES)

#    set(CMAKE_REQUIRED_INCLUDES ${BOOST_LIBRARY_INCLUDES})
#    set(CMAKE_REQUIRED_FLAGS "${boost_atomic_sse41_cflags}")
#    check_cxx_source_compiles("#include <${CMAKE_CURRENT_SOURCE_DIR}/config/has_sse41.cpp>" BOOST_ATOMIC_COMPILER_HAS_SSE41)
#    unset(CMAKE_REQUIRED_FLAGS)
#    unset(CMAKE_REQUIRED_INCLUDES)

#    if (BOOST_ATOMIC_COMPILER_HAS_SSE2)
#        set(boost_atomic_sources_sse2 src/find_address_sse2.cpp)
#        set_source_files_properties(${boost_atomic_sources_sse2} PROPERTIES COMPILE_FLAGS "${boost_atomic_sse2_cflags}")
#        set(boost_atomic_sources ${boost_atomic_sources} ${boost_atomic_sources_sse2})
#    endif()

#    if (BOOST_ATOMIC_COMPILER_HAS_SSE41)
#        set(boost_atomic_sources_sse41 src/find_address_sse41.cpp)
#        set_source_files_properties(${boost_atomic_sources_sse41} PROPERTIES COMPILE_FLAGS "${boost_atomic_sse41_cflags}")
#        set(boost_atomic_sources ${boost_atomic_sources} ${boost_atomic_sources_sse41})
#    endif()
#endif()



#if (WIN32)
#    # Note: We can't use the Boost::library targets here as they may not yet be included by the superproject when this CMakeLists.txt is included.
#    set(CMAKE_REQUIRED_INCLUDES ${BOOST_LIBRARY_INCLUDES})
#    set(CMAKE_REQUIRED_LIBRARIES synchronization)
#    check_cxx_source_compiles("#include <${CMAKE_CURRENT_SOURCE_DIR}/config/has_synchronization.cpp>" BOOST_ATOMIC_HAS_SYNCHRONIZATION)
#    unset(CMAKE_REQUIRED_LIBRARIES)
#    unset(CMAKE_REQUIRED_INCLUDES)
#endif()

#add_library(boost_atomic ${boost_atomic_sources})
#add_library(Boost::atomic ALIAS boost_atomic)

#target_include_directories(boost_atomic PUBLIC include)
#target_include_directories(boost_atomic PRIVATE src)

#target_link_libraries(boost_atomic
#    PUBLIC
#        Boost::assert
#        Boost::config
#        Boost::static_assert
#        Boost::type_traits
#    PRIVATE
#        Boost::align
#        Boost::predef
#        Boost::preprocessor

#        Threads::Threads
#)

#if (WIN32)
#    target_link_libraries(boost_atomic
#        PUBLIC
#            Boost::winapi
#    )

#    if (BOOST_ATOMIC_HAS_SYNCHRONIZATION)
#        target_link_libraries(boost_atomic PRIVATE synchronization)
#    endif()
#endif()

#target_compile_definitions(boost_atomic
#    PUBLIC
#        BOOST_ATOMIC_NO_LIB
#    PRIVATE
#        BOOST_ATOMIC_SOURCE
#)

#    target_compile_definitions(boost_atomic PUBLIC )

#if (BOOST_ATOMIC_COMPILER_HAS_SSE2)
#    target_compile_definitions(boost_atomic PRIVATE BOOST_ATOMIC_USE_SSE2)
#endif()
#if (BOOST_ATOMIC_COMPILER_HAS_SSE41)
#    target_compile_definitions(boost_atomic PRIVATE BOOST_ATOMIC_USE_SSE41)
#endif()


boost_libs += static_library('boost-atomic', BOOST_ATOMIC_SOURCES, include_directories: [boost_inc, include_directories('src')], cpp_args: BOOST_ATOMIC_ARGS)
