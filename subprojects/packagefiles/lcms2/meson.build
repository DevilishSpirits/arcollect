# Modifications of HarfBuzz meson.build for Arcollect
project('Little-CMS', 'c')

cc = meson.get_compiler('c')

lcms2_srcs = [
  'src/cmsalpha.c',
  'src/cmscgats.c',
  'src/cmserr.c',
  'src/cmsgmt.c',
  'src/cmsintrp.c',
  'src/cmsio1.c',
  'src/cmsmd5.c',
  'src/cmsnamed.c',
  'src/cmspack.c',
  'src/cmsplugin.c',
  'src/cmssamp.c',
  'src/cmstypes.c',
  'src/cmswtpnt.c',
  'src/cmscam02.c',
  'src/cmscnvrt.c',
  'src/cmsgamma.c',
  'src/cmshalf.c',
  'src/cmsio0.c',
  'src/cmslut.c',
  'src/cmsmtrx.c',
  'src/cmsopt.c',
  'src/cmspcs.c',
  'src/cmsps2.c',
  'src/cmssm.c',
  'src/cmsvirt.c',
  'src/cmsxform.c',
]

inc_dirs = include_directories('include')

version = cc.get_define('LCMS_VERSION', include_directories: inc_dirs, prefix: '#include <lcms2.h>').to_int()
version_major =  version/1000
version_minor = (version%1000)/10
version_patch =  version%10
version = version_major.to_string()+'.'+version_minor.to_string()+'.'+version_patch.to_string()

deps = [
  cc.find_library('m', required: false),
  dependency('threads'),
]

global_cargs = [
	'-DCMS_NO_REGISTER_KEYWORD'
]
cargs = [
	global_cargs,
	'-DHasTHREADS=1',
]

# Check for threadsafe variants of gmtime
if cc.has_function('gmtime_r', prefix: '#include <time.h>')
  cargs += '-DHAVE_GMTIME_R=1'
elif cc.has_function('gmtime_s', prefix: '#include <time.h>')
  cargs += '-DHAVE_GMTIME_S=1'
endif

liblcms2_lib = static_library('lcms2', lcms2_srcs,
  include_directories : inc_dirs,
  dependencies: deps,
  c_args: cargs,
  install: false,
  build_by_default: false,
)

lcms2_dep = declare_dependency(
  link_with : liblcms2_lib,
  version: version,
  compile_args: global_cargs,
  include_directories : inc_dirs
)
