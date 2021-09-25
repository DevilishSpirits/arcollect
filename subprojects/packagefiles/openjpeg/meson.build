
project('openjp2', 'c', 'cpp', version : '2.3.1',
        meson_version : '>= 0.54.99',
          default_options : [
          'buildtype=debugoptimized',
          'c_std=gnu89'
        ]
        )

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
host_system = host_machine.system()
host_cpu = host_machine.cpu_family()

fs = import('fs')

openjpeg_version_major = '2'
openjpeg_version_minor = '3'
openjpeg_version_build = '1'
openjpeg_package_version = '@0@.@1@.@2@'.format(openjpeg_version_major, openjpeg_version_minor, openjpeg_version_build)


m_dep = cc.find_library('m')
thread_dep = dependency('threads')


# clock_gettime might require -rt, or it might not. find out
if not cc.has_function('clock_gettime', prefix : '#define _GNU_SOURCE\n#include <time.h>')
  # XXX: untested
  rt_dep = cc.find_library('rt')
else
  rt_dep = []
endif

wxwidgets_dep = dependency('wxWidgets', required: get_option('build_viewer'))

subdir('thirdparty')

if host_system == 'windows'
  if get_option('default_library') == 'static'
    add_project_arguments(['-DOPJ_STATIC'], language: 'c')
  else
    add_project_arguments(['-DOPJ_EXPORTS'], language: 'c')
  endif
endif

#-----------------------------------------------------------------------------
# Big endian test:
# test_big_endian('OPJ_BIG_ENDIAN')

#-----------------------------------------------------------------------------
# OpenJPEG build configuration options.
#executable_output_path = [openjpeg_binary_dir, '/bin', 'CACHE', 'PATH', 'Single output directory for building all executables.']
#library_output_path = [openjpeg_binary_dir, '/bin', 'CACHE', 'PATH', 'Single output directory for building all libraries.']
# mark_as_advanced(['LIBRARY_OUTPUT_PATH', 'EXECUTABLE_OUTPUT_PATH'])
#-----------------------------------------------------------------------------
# configure name mangling to allow multiple libraries to coexist
# peacefully

#if fs.exists('openjpeg_mangle.h.in')
#  mangle_prefix = openjpeg_library_name
  # configure_file([cmake_current_source_dir, '/openjpeg_mangle.h.in', cmake_current_binary_dir, '/openjpeg_mangle.h', '@ONLY'])
#endif
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# Build Library
subdir('src/lib')

#-----------------------------------------------------------------------------
# build our version of astyle
#with_astyle = ['FALSE', 'CACHE', 'BOOL', 'If you plan to contribute you should reindent with scripts/prepare-commit.sh (using 'our' astyle)']
