jconfig_h = configure_file(input: 'jconfig.h.in',
  output: 'jconfig.h',
  format: 'cmake@',
  configuration: cdata)

configure_file(input: 'jconfigint.h.in',
  output: 'jconfigint.h',
  format: 'cmake@',
  configuration: cdata)

configure_file(input: 'jversion.h.in',
  output: 'jversion.h',
  format: 'cmake@',
  configuration: cdata)

jpeg16_sources = [
  'jcapistd.c', 'jccolor.c', 'jcdiffct.c', 'jclossls.c', 'jcmainct.c',
  'jcprepct.c', 'jcsample.c', 'jdapistd.c', 'jdcolor.c', 'jddiffct.c',
  'jdlossls.c', 'jdmainct.c', 'jdpostct.c', 'jdsample.c', 'jutils.c',
]
jpeg12_sources = [
  jpeg16_sources, 'jccoefct.c', 'jcdctmgr.c', 'jdcoefct.c', 'jddctmgr.c',
  'jdmerge.c', 'jfdctfst.c', 'jfdctint.c', 'jidctflt.c', 'jidctfst.c',
  'jidctint.c', 'jidctred.c', 'jquant1.c', 'jquant2.c',
]
sources = [
  jpeg12_sources, 'jcapimin.c', 'jchuff.c', 'jcicc.c', 'jcinit.c',
  'jclhuff.c', 'jcmarker.c', 'jcmaster.c', 'jcomapi.c', 'jcparam.c',
  'jcphuff.c', 'jctrans.c', 'jdapimin.c', 'jdatadst.c', 'jdatasrc.c',
  'jdhuff.c', 'jdicc.c', 'jdinput.c', 'jdlhuff.c', 'jdmarker.c',
  'jdmaster.c', 'jdphuff.c', 'jdtrans.c', 'jerror.c', 'jfdctflt.c',
  'jmemmgr.c', 'jmemnobs.c', 'jpeg_nbits.c',
  # TODO: `with_arith_dec` / `with_arith_enc` only.
  'jaricom.c',
  # TODO: `with_arith_enc` only
  'jcarith.c',
  # TODO: `with_arith_dec` only
  'jdarith.c',
]

jpeg12 = static_library('jpeg12', jpeg12_sources,
  c_args: ['-DBITS_IN_JSAMPLE=12'],
  pic: true,
)
jpeg16 = static_library('jpeg16', jpeg16_sources,
  c_args: ['-DBITS_IN_JSAMPLE=16'],
  pic: true,
)
jpeg = static_library('jpeg', sources,
  link_whole: [jpeg12, jpeg16, simd],
  install: false,
)

jpeg_dep = declare_dependency(
  include_directories: incdir,
  link_with: jpeg
)

meson.override_dependency('libjpeg', jpeg_dep)
