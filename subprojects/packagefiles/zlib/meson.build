project('zlib', 'c', license : 'zlib')

cc = meson.get_compiler('c')

link_args = []
compile_args = []
if cc.get_id() == 'msvc'
  add_project_arguments('-D_CRT_SECURE_NO_DEPRECATE',
    '-D_CRT_NONSTDC_NO_DEPRECATE', language : 'c')
else
  # Don't spam consumers of this wrap with these warnings
  compile_args += cc.get_supported_arguments(['-Wno-implicit-fallthrough',
                                              '-Wno-implicit-function-declaration'])
  if cc.get_id() == 'gcc' and host_machine.system() != 'windows'
    vflag = '-Wl,--version-script,@0@/zlib.map'.format(meson.current_source_dir())
    link_args += [vflag]
  endif
endif

src = files([
  'adler32.c',
  'crc32.c',
  'deflate.c',
  'infback.c',
  'inffast.c',
  'inflate.c',
  'inftrees.c',
  'trees.c',
  'zutil.c',
  'compress.c',
  'uncompr.c',
  'gzclose.c',
  'gzlib.c',
  'gzread.c',
  'gzwrite.c'])

headers = files(['zconf.h', 'zlib.h'])

if host_machine.system() == 'windows'
  win = import('windows')
  win_args = []
  if cc.get_id() != 'msvc'
    win_args += '-DGCC_WINDRES'
  endif
  src += win.compile_resources('win32/zlib1.rc', args : win_args)
endif

zlib = library('z', src,
  c_args : compile_args,
  link_args : link_args,
  vs_module_defs : 'win32/zlib.def',
  build_by_default: false,
  install : false)

incdir = include_directories('.')

zlib_ver = cc.get_define('ZLIB_VERSION', include_directories: incdir, prefix: '#include <zlib.h>').strip('"')

zlib_dep = declare_dependency(
  link_with : zlib,
  include_directories : incdir,
  version: zlib_ver,
  )

zlib_source_dir = meson.current_source_dir()
