cappdata = configuration_data()

# 3rd party libs
#------------

cappdata.set('OPJ_HAVE_LIBPNG', 1)
cappdata.set('OPJ_HAVE_PNG_H', 1)
libpng_dep = dependency('libpng')

cappdata.set('OPJ_HAVE_LIBTIFF', 1)
cappdata.set('OPJ_HAVE_TIFF_H', 1)
libtiff_dep = dependency('libtiff-4')

# Try to find lcms2 first
cappdata.set('OPJ_HAVE_LIBLCMS2', 1)
cappdata.set('OPJ_HAVE_LCMS2_H', 1)
#liblcms2_dep = dependency('lcms2', required : false)
#if liblcms2_dep.found()
#  message('STATUS: Your system seems to have a lcms2 lib available, we will use it instead of thirdparty folder')
#else
  subdir('liblcms2')
  message('STATUS: tiff not found, We will build libtiff from thirdparty folder')
#endif

#------------TODO
#if get_option('WITH_ASTYLE')
#  subdir('astyle')
#endif

