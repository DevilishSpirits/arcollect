project('libpng', version: '1.6.37')
# CMakeLists.txt

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

# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

#cmake_minimum_required(VERSION 3.1)
#cmake_policy(VERSION 3.1)

#project(libpng C ASM)
#enable_testing()

#set(PNGLIB_MAJOR 1)
#set(PNGLIB_MINOR 6)
#set(PNGLIB_RELEASE 37)
#set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
#set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

#include(GNUInstallDirs)

cc = meson.get_compiler('c')
zlib_dep = dependency('zlib', method: 'system')
m_dep = cc.find_library('m', required: false)
# needed packages

# Many more configuration options could be added here
#option(PNG_HARDWARE_OPTIMIZATIONS "Enable hardware optimizations" ON)

#set(PNG_PREFIX "" CACHE STRING "Prefix to add to the API function names")

#if(PNG_HARDWARE_OPTIMIZATIONS)

## set definitions and sources for arm
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" OR
#  CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
#  set(PNG_ARM_NEON_POSSIBLE_VALUES check on off)
#  set(PNG_ARM_NEON "check" CACHE STRING "Enable ARM NEON optimizations:
#     check: (default) use internal checking code;
#     off: disable the optimizations;
#     on: turn on unconditionally.")
#  set_property(CACHE PNG_ARM_NEON PROPERTY STRINGS
#     ${PNG_ARM_NEON_POSSIBLE_VALUES})
#  list(FIND PNG_ARM_NEON_POSSIBLE_VALUES ${PNG_ARM_NEON} index)
#  if(index EQUAL -1)
#    message(FATAL_ERROR
#            "PNG_ARM_NEON must be one of [${PNG_ARM_NEON_POSSIBLE_VALUES}]")
#  elseif(NOT ${PNG_ARM_NEON} STREQUAL "off")
#    set(libpng_arm_sources
#      arm/arm_init.c
#      arm/filter_neon.S
#      arm/filter_neon_intrinsics.c
#      arm/palette_neon_intrinsics.c)

#    if(${PNG_ARM_NEON} STREQUAL "on")
#      add_definitions(-DPNG_ARM_NEON_OPT=2)
#    elseif(${PNG_ARM_NEON} STREQUAL "check")
#      add_definitions(-DPNG_ARM_NEON_CHECK_SUPPORTED)
#    endif()
#  else()
#    add_definitions(-DPNG_ARM_NEON_OPT=0)
#  endif()
#endif()

## set definitions and sources for powerpc
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "^powerpc*" OR
#   CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc64*")
#  set(PNG_POWERPC_VSX_POSSIBLE_VALUES on off)
#  set(PNG_POWERPC_VSX "on" CACHE STRING "Enable POWERPC VSX optimizations:
#     off: disable the optimizations.")
#  set_property(CACHE PNG_POWERPC_VSX PROPERTY STRINGS
#     ${PNG_POWERPC_VSX_POSSIBLE_VALUES})
#  list(FIND PNG_POWERPC_VSX_POSSIBLE_VALUES ${PNG_POWERPC_VSX} index)
#  if(index EQUAL -1)
#    message(FATAL_ERROR
#            "PNG_POWERPC_VSX must be one of [${PNG_POWERPC_VSX_POSSIBLE_VALUES}]")
#  elseif(NOT ${PNG_POWERPC_VSX} STREQUAL "off")
#    set(libpng_powerpc_sources
#      powerpc/powerpc_init.c
#      powerpc/filter_vsx_intrinsics.c)
#    if(${PNG_POWERPC_VSX} STREQUAL "on")
#      add_definitions(-DPNG_POWERPC_VSX_OPT=2)
#    endif()
#  else()
#    add_definitions(-DPNG_POWERPC_VSX_OPT=0)
#  endif()
#endif()

## set definitions and sources for intel
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "^i?86" OR
#   CMAKE_SYSTEM_PROCESSOR MATCHES "^x86_64*")
#  set(PNG_INTEL_SSE_POSSIBLE_VALUES on off)
#  set(PNG_INTEL_SSE "on" CACHE STRING "Enable INTEL_SSE optimizations:
#     off: disable the optimizations")
#  set_property(CACHE PNG_INTEL_SSE PROPERTY STRINGS
#     ${PNG_INTEL_SSE_POSSIBLE_VALUES})
#  list(FIND PNG_INTEL_SSE_POSSIBLE_VALUES ${PNG_INTEL_SSE} index)
#  if(index EQUAL -1)
#    message(FATAL_ERROR
#            "PNG_INTEL_SSE must be one of [${PNG_INTEL_SSE_POSSIBLE_VALUES}]")
#  elseif(NOT ${PNG_INTEL_SSE} STREQUAL "off")
#    set(libpng_intel_sources
#      intel/intel_init.c
#      intel/filter_sse2_intrinsics.c)
#    if(${PNG_INTEL_SSE} STREQUAL "on")
#      add_definitions(-DPNG_INTEL_SSE_OPT=1)
#    endif()
#  else()
#    add_definitions(-DPNG_INTEL_SSE_OPT=0)
#  endif()
#endif()

## set definitions and sources for MIPS
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "mipsel*" OR
#   CMAKE_SYSTEM_PROCESSOR MATCHES "mips64el*")
#  set(PNG_MIPS_MSA_POSSIBLE_VALUES on off)
#  set(PNG_MIPS_MSA "on" CACHE STRING "Enable MIPS_MSA optimizations:
#     off: disable the optimizations")
#  set_property(CACHE PNG_MIPS_MSA PROPERTY STRINGS
#     ${PNG_MIPS_MSA_POSSIBLE_VALUES})
#  list(FIND PNG_MIPS_MSA_POSSIBLE_VALUES ${PNG_MIPS_MSA} index)
#  if(index EQUAL -1)
#    message(FATAL_ERROR
#            "PNG_MIPS_MSA must be one of [${PNG_MIPS_MSA_POSSIBLE_VALUES}]")
#  elseif(NOT ${PNG_MIPS_MSA} STREQUAL "off")
#    set(libpng_mips_sources
#      mips/mips_init.c
#      mips/filter_msa_intrinsics.c)
#    if(${PNG_MIPS_MSA} STREQUAL "on")
#      add_definitions(-DPNG_MIPS_MSA_OPT=2)
#    endif()
#  else()
#    add_definitions(-DPNG_MIPS_MSA_OPT=0)
#  endif()
#endif()

#else(PNG_HARDWARE_OPTIMIZATIONS)

## set definitions and sources for arm
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" OR
#   CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
#  add_definitions(-DPNG_ARM_NEON_OPT=0)
#endif()

## set definitions and sources for powerpc
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "^powerpc*" OR
#   CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc64*")
#  add_definitions(-DPNG_POWERPC_VSX_OPT=0)
#endif()

## set definitions and sources for intel
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "^i?86" OR
#   CMAKE_SYSTEM_PROCESSOR MATCHES "^x86_64*")
#  add_definitions(-DPNG_INTEL_SSE_OPT=0)
#endif()
#
## set definitions and sources for MIPS
#if(CMAKE_SYSTEM_PROCESSOR MATCHES "mipsel*" OR
#   CMAKE_SYSTEM_PROCESSOR MATCHES "mips64el*")
#  add_definitions(-DPNG_MIPS_MSA_OPT=0)
#endif()
#
#endif(PNG_HARDWARE_OPTIMIZATIONS)

#find_program(AWK NAMES gawk awk)

#include_directories(${CMAKE_CURRENT_BINARY_DIR})

#if(NOT AWK OR ANDROID)
#  # No awk available to generate sources; use pre-built pnglibconf.h
	configure_file(input: 'scripts/pnglibconf.h.prebuilt', output: 'pnglibconf.h', copy: true)
#  add_custom_target(genfiles) # Dummy
#else()
#  include(CMakeParseArguments)
#  # Generate .chk from .out with awk
#  # generate_chk(INPUT inputfile OUTPUT outputfile [DEPENDS dep1 [dep2...]])
#  function(generate_chk)
#    set(options)
#    set(oneValueArgs INPUT OUTPUT)
#    set(multiValueArgs DEPENDS)
#    cmake_parse_arguments(_GC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
#    if(NOT _GC_INPUT)
#      message(FATAL_ERROR "generate_chk: Missing INPUT argument")
#    endif()
#    if(NOT _GC_OUTPUT)
#      message(FATAL_ERROR "generate_chk: Missing OUTPUT argument")
#    endif()

#    add_custom_command(OUTPUT "${_GC_OUTPUT}"
#                       COMMAND "${CMAKE_COMMAND}"
#                               "-DINPUT=${_GC_INPUT}"
#                               "-DOUTPUT=${_GC_OUTPUT}"
#                               -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/genchk.cmake"
#                       DEPENDS "${_GC_INPUT}" ${_GC_DEPENDS}
#                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
#  endfunction()

#  # Generate .out from .c with awk
#  # generate_out(INPUT inputfile OUTPUT outputfile [DEPENDS dep1 [dep2...]])
#  function(generate_out)
#    set(options)
#    set(oneValueArgs INPUT OUTPUT)
#    set(multiValueArgs DEPENDS)
#    cmake_parse_arguments(_GO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
#    if(NOT _GO_INPUT)
#      message(FATAL_ERROR "generate_out: Missing INPUT argument")
#    endif()
#    if(NOT _GO_OUTPUT)
#      message(FATAL_ERROR "generate_out: Missing OUTPUT argument")
#    endif()

#    add_custom_command(OUTPUT "${_GO_OUTPUT}"
#                       COMMAND "${CMAKE_COMMAND}"
#                               "-DINPUT=${_GO_INPUT}"
#                               "-DOUTPUT=${_GO_OUTPUT}"
#                               -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/genout.cmake"
#                       DEPENDS "${_GO_INPUT}" ${_GO_DEPENDS}
#                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
#  endfunction()

#  # Generate specific source file with awk
#  # generate_source(OUTPUT outputfile [DEPENDS dep1 [dep2...]])
#  function(generate_source)
#    set(options)
#    set(oneValueArgs OUTPUT)
#    set(multiValueArgs DEPENDS)
#    cmake_parse_arguments(_GSO "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
#    if(NOT _GSO_OUTPUT)
#      message(FATAL_ERROR "generate_source: Missing OUTPUT argument")
#    endif()

#    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_GSO_OUTPUT}"
#                       COMMAND "${CMAKE_COMMAND}"
#                               "-DOUTPUT=${_GSO_OUTPUT}"
#                               -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/gensrc.cmake"
#                       DEPENDS ${_GSO_DEPENDS}
#                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
#  endfunction()

#  # Copy file
#  function(generate_copy source destination)
#    add_custom_command(OUTPUT "${destination}"
#                       COMMAND "${CMAKE_COMMAND}" -E remove "${destination}"
#                       COMMAND "${CMAKE_COMMAND}" -E copy "${source}"
#                                                          "${destination}"
#                       DEPENDS "${source}")
#  endfunction()

#  # Generate scripts/pnglibconf.h
#  generate_source(OUTPUT "scripts/pnglibconf.c"
#                  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.dfa"
#                          "${CMAKE_CURRENT_SOURCE_DIR}/scripts/options.awk"
#                          "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h")

#  # Generate pnglibconf.c
#  generate_source(OUTPUT "pnglibconf.c"
#                  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.dfa"
#                          "${CMAKE_CURRENT_SOURCE_DIR}/scripts/options.awk"
#                          "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h")

#  if(PNG_PREFIX)
#    set(PNGLIBCONF_H_EXTRA_DEPENDS
#        "${CMAKE_CURRENT_BINARY_DIR}/scripts/prefix.out"
#        "${CMAKE_CURRENT_SOURCE_DIR}/scripts/macro.lst")
#    set(PNGPREFIX_H_EXTRA_DEPENDS
#        "${CMAKE_CURRENT_BINARY_DIR}/scripts/intprefix.out")
#  endif()

#  generate_out(INPUT "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.c"
#               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out")

#  # Generate pnglibconf.h
#  generate_source(OUTPUT "pnglibconf.h"
#                  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out"
#                          ${PNGLIBCONF_H_EXTRA_DEPENDS})

#  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/intprefix.c"
#               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/intprefix.out"
#               DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h")

#  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/prefix.c"
#               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/prefix.out"
#               DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/png.h"
#                       "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h"
#                       "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out")

#  # Generate pngprefix.h
#  generate_source(OUTPUT "pngprefix.h"
#                  DEPENDS ${PNGPREFIX_H_EXTRA_DEPENDS})

#  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/sym.c"
#               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/sym.out"
#               DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h")

#  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/symbols.c"
#               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.out"
#               DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/png.h"
#                       "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h"
#                       "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pnglibconf.h.prebuilt")

#  generate_out(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/vers.c"
#               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/vers.out"
#               DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/png.h"
#                       "${CMAKE_CURRENT_SOURCE_DIR}/pngconf.h"
#                       "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h")

#  generate_chk(INPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.out"
#               OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.chk"
#               DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/checksym.awk"
#                       "${CMAKE_CURRENT_SOURCE_DIR}/scripts/symbols.def")

#  add_custom_target(symbol-check DEPENDS
#                    "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.chk")

#  generate_copy("${CMAKE_CURRENT_BINARY_DIR}/scripts/sym.out"
#                "${CMAKE_CURRENT_BINARY_DIR}/libpng.sym")
#  generate_copy("${CMAKE_CURRENT_BINARY_DIR}/scripts/vers.out"
#                "${CMAKE_CURRENT_BINARY_DIR}/libpng.vers")
#  add_custom_target("genprebuilt"
#                    COMMAND "${CMAKE_COMMAND}"
#                            "-DOUTPUT=scripts/pnglibconf.h.prebuilt"
#                            -P "${CMAKE_CURRENT_BINARY_DIR}/scripts/gensrc.cmake"
#                    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

#  # A single target handles generation of all generated files.  If
#  # they are depended upon separately by multiple targets, this
#  # confuses parallel make (it would require a separate top-level
#  # target for each file to track the dependencies properly).
#  add_custom_target(genfiles DEPENDS
#    "${CMAKE_CURRENT_BINARY_DIR}/libpng.sym"
#    "${CMAKE_CURRENT_BINARY_DIR}/libpng.vers"
#    "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.c"
#    "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h"
#    "${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.out"
#    "${CMAKE_CURRENT_BINARY_DIR}/pngprefix.h"
#    "${CMAKE_CURRENT_BINARY_DIR}/scripts/intprefix.out"
#    "${CMAKE_CURRENT_BINARY_DIR}/scripts/pnglibconf.c"
#    "${CMAKE_CURRENT_BINARY_DIR}/scripts/prefix.out"
#    "${CMAKE_CURRENT_BINARY_DIR}/scripts/sym.out"
#    "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.chk"
#    "${CMAKE_CURRENT_BINARY_DIR}/scripts/symbols.out"
#    "${CMAKE_CURRENT_BINARY_DIR}/scripts/vers.out")
#endif(NOT AWK OR ANDROID)

# OUR SOURCES
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
#if(AWK AND NOT ANDROID)
#  list(APPEND libpng_private_hdrs "${CMAKE_CURRENT_BINARY_DIR}/pngprefix.h")
#endif()
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

# NOW BUILD OUR TARGET
png_incs   = include_directories('.')
png_static = static_library('png_static', libpng_sources, dependencies: [zlib_dep, m_dep], include_directories: png_incs)

libpng_dep = declare_dependency(link_with: png_static, dependencies: [zlib_dep, m_dep], include_directories: png_incs)
png_dep    = libpng_dep
# Create source generation scripts.
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/genchk.cmake.in
#               ${CMAKE_CURRENT_BINARY_DIR}/scripts/genchk.cmake @ONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/genout.cmake.in
#               ${CMAKE_CURRENT_BINARY_DIR}/scripts/genout.cmake @ONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/gensrc.cmake.in
#               ${CMAKE_CURRENT_BINARY_DIR}/scripts/gensrc.cmake @ONLY)
