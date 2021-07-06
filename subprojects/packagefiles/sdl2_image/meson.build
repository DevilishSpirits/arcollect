project('sdl2-image', 'c', version: '2.0.5')
config_h = configuration_data()
#dnl Process this file with autoconf to produce a configure script.
#AC_INIT(README.txt)
#
#dnl Set various version strings - taken gratefully from the GTk sources
#
## Making releases:
##   MICRO_VERSION += 1;
##   INTERFACE_AGE += 1;
##   BINARY_AGE += 1;
## if any functions have been added, set INTERFACE_AGE to 0.
## if backwards compatibility has been broken,
## set BINARY_AGE and INTERFACE_AGE to 0.
#
#MAJOR_VERSION=2
#MINOR_VERSION=0
#MICRO_VERSION=5
#INTERFACE_AGE=3
#BINARY_AGE=5
#VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION
#
#AC_SUBST(MAJOR_VERSION)
#AC_SUBST(MINOR_VERSION)
#AC_SUBST(MICRO_VERSION)
#AC_SUBST(INTERFACE_AGE)
#AC_SUBST(BINARY_AGE)
#AC_SUBST(VERSION)
#
## libtool versioning
#LT_INIT([win32-dll])
#
#LT_RELEASE=$MAJOR_VERSION.$MINOR_VERSION
#LT_CURRENT=`expr $MICRO_VERSION - $INTERFACE_AGE`
#LT_REVISION=$INTERFACE_AGE
#LT_AGE=`expr $BINARY_AGE - $INTERFACE_AGE`
#
#AC_SUBST(LT_RELEASE)
#AC_SUBST(LT_CURRENT)
#AC_SUBST(LT_REVISION)
#AC_SUBST(LT_AGE)
#
#dnl Detect the canonical build and host environments
#AC_CANONICAL_HOST
#
#dnl Setup for automake
#AM_INIT_AUTOMAKE(SDL2_image, $VERSION)
#
#dnl Check for tools
#AC_PROG_LIBTOOL
#AC_PROG_CC
#AC_PROG_OBJC
#AC_C_INLINE
#AC_PROG_INSTALL
#AC_PROG_MAKE_SET
#if test -z "$host_alias"; then
#    hostaliaswindres=
#else
#    hostaliaswindres="$host_alias-windres"
#fi
#AC_CHECK_PROGS(WINDRES, [windres $hostaliaswindres $host_os-windres])
#
#case "$host" in
#    *-*-beos*)
#        ac_default_prefix=/boot/develop/tools/gnupro
#        ;;
#    *-*-cygwin* | *-*-mingw32*)
#        if test "$build" != "$host"; then # cross-compiling
#            # Default cross-compile location
#            ac_default_prefix=/usr/local/cross-tools/$host
#        else
#            # Look for the location of the tools and install there
#            if test "$BUILD_PREFIX" != ""; then
#                ac_default_prefix=$BUILD_PREFIX
#            fi
#        fi
#        if test x$WINDRES != x; then
#            use_version_rc=true
#        fi
#        ;;
#    *-*-darwin*)
#        AC_ARG_ENABLE([imageio], [AC_HELP_STRING([--enable-imageio], [use native Mac OS X frameworks for loading images [default=yes]])],
#                      [], [enable_imageio=yes])
#        dnl Show a message when we use ImageIO support so it's not a surprise
#        AC_MSG_CHECKING([for ImageIO support])
#        AC_MSG_RESULT($enable_imageio)
#        if test x$enable_imageio = xyes; then
#            IMG_LIBS="-Wl,-framework,ApplicationServices -lobjc $IMG_LIBS"
#        else
#            CFLAGS="$CFLAGS -DSDL_IMAGE_USE_COMMON_BACKEND"
#        fi
#        ;;
#esac
#AM_CONDITIONAL(USE_IMAGEIO, test x$enable_imageio = xyes)
#AM_CONDITIONAL(USE_VERSION_RC, test x$use_version_rc = xtrue)
#
#dnl set this to use on systems that use lib64 instead of lib
#base_libdir=`echo \${libdir} | sed 's/.*\/\(.*\)/\1/; q'`
#
#dnl Function to find a library in the compiler search path
#find_lib()
#{
#    gcc_bin_path=[`$CC -print-search-dirs 2>/dev/null | fgrep programs: | sed 's/[^=]*=\(.*\)/\1/' | sed 's/:/ /g'`]
#    gcc_lib_path=[`$CC -print-search-dirs 2>/dev/null | fgrep libraries: | sed 's/[^=]*=\(.*\)/\1/' | sed 's/:/ /g'`]
#    env_lib_path=[`echo $LIBS $LDFLAGS $* | sed 's/-L[ ]*//g'`]
#    if test "$cross_compiling" = yes; then
#        host_lib_path=""
#    else
#        host_lib_path="/usr/$base_libdir /usr/local/$base_libdir"
#    fi
#    for path in $env_lib_path $gcc_bin_path $gcc_lib_path $host_lib_path; do
#        lib=[`ls -- $path/$1 2>/dev/null | sed -e '/\.so\..*\./d' -e 's,.*/,,' | sort | tail -1`]
#        if test x$lib != x; then
#            echo $lib
#            return
#        fi
#    done
#}
#
#dnl Check for SDL
#AC_SUBST(SDL_VERSION)
deps = [
	dependency('sdl2', version: '>=2.0.8'),
]

jpg_dep = dependency('libjpeg', required: get_option('enable_jpg'))
if jpg_dep.found()
	add_project_arguments('-DLOAD_JPG', language: 'c')
	deps += jpg_dep
endif

png_dep = dependency('libpng', required: get_option('enable_png'))
if jpg_dep.found()
	add_project_arguments('-DLOAD_PNG', language: 'c')
	deps += png_dep
endif

tiff_dep = dependency('libtiff-4', required: get_option('enable_tif'))
if jpg_dep.found()
	add_project_arguments('-DLOAD_TIFF', language: 'c')
	deps += png_dep
endif

webp_dep = dependency('libwebp', required: get_option('enable_webp'))
if webp_dep.found()
	add_project_arguments('-DLOAD_WEBP', language: 'c')
	deps += webp_dep
endif

if get_option('enable_bmp').auto() or get_option('enable_bmp').enabled() #get_option('enable_bmp').allowed()
	add_project_arguments('-DLOAD_BMP', language: 'c')
endif
if get_option('enable_gif').auto() or get_option('enable_gif').enabled() #get_option('enable_gif').allowed()
	add_project_arguments('-DLOAD_GIF', language: 'c')
endif
if get_option('enable_lbm').auto() or get_option('enable_lbm').enabled() #get_option('enable_lbm').allowed()
	add_project_arguments('-DLOAD_LBM', language: 'c')
endif
if get_option('enable_pcx').auto() or get_option('enable_pcx').enabled() #get_option('enable_pcx').allowed()
	add_project_arguments('-DLOAD_PCX', language: 'c')
endif
if get_option('enable_pnm').auto() or get_option('enable_pnm').enabled() #get_option('enable_pnm').allowed()
	add_project_arguments('-DLOAD_PNM', language: 'c')
endif
if get_option('enable_svg').auto() or get_option('enable_svg').enabled() #get_option('enable_svg').allowed()
	add_project_arguments('-DLOAD_SVG', language: 'c')
endif
if get_option('enable_tga').auto() or get_option('enable_tga').enabled() #get_option('enable_tga').allowed()
	add_project_arguments('-DLOAD_TGA', language: 'c')
endif
if get_option('enable_xcf').auto() or get_option('enable_xcf').enabled() #get_option('enable_xcf').allowed()
	add_project_arguments('-DLOAD_XCF', language: 'c')
endif
if get_option('enable_xpm').auto() or get_option('enable_xpm').enabled() #get_option('enable_xpm').allowed()
	add_project_arguments('-DLOAD_XPM', language: 'c')
endif
if get_option('enable_xv').auto() or get_option('enable_xv').enabled() #get_option('enable_xv').allowed()
	add_project_arguments('-DLOAD_XV', language: 'c')
endif
#
#if test x$enable_webp = xyes -a x$have_webp_hdr = xyes -a x$have_webp_lib = xyes; then
#    CFLAGS="$LIBWEBP_CFLAGS $CFLAGS"
#    if test x$enable_webp_shared = xyes && test x$webp_lib != x; then
#        echo "-- dynamic libwebp -> $webp_lib"
#        AC_DEFINE_UNQUOTED(LOAD_WEBP_DYNAMIC, "$webp_lib")
#    else
#        IMG_LIBS="$LIBWEBP_LIBS $IMG_LIBS"
#    fi
#fi
#
#if test x$enable_tif = xyes -a x$have_tif_hdr = xyes -a x$have_tif_lib = xyes; then
#    if test x$enable_tif_shared = xyes && test x$tif_lib != x; then
#        echo "-- dynamic libtiff -> $tif_lib"
#        AC_DEFINE_UNQUOTED(LOAD_TIF_DYNAMIC, "$tif_lib")
#    else
#        if test x$have_libjpeg = xyes; then
#            # Disable dynamic jpeg since we're linking it explicitly
#            jpg_lib=''
#        fi
#        IMG_LIBS="-ltiff -lz $IMG_LIBS"
#    fi
#fi
#if test x$enable_jpg = xyes -a x$have_jpg_hdr = xyes -a x$have_jpg_lib = xyes; then
#    if test x$enable_jpg_shared = xyes && test x$jpg_lib != x; then
#        echo "-- dynamic libjpeg -> $jpg_lib"
#        AC_DEFINE_UNQUOTED(LOAD_JPG_DYNAMIC, "$jpg_lib")
#    else
#        IMG_LIBS="-ljpeg $IMG_LIBS"
#    fi
#fi
#if test x$enable_png = xyes -a x$have_png_hdr = xyes -a x$have_png_lib = xyes; then
#    CFLAGS="$LIBPNG_CFLAGS $CFLAGS"
#    if test x$enable_png_shared = xyes && test x$png_lib != x; then
#        echo "-- dynamic libpng -> $png_lib"
#        AC_DEFINE_UNQUOTED(LOAD_PNG_DYNAMIC, "$png_lib")
#    else
#        IMG_LIBS="$LIBPNG_LIBS $IMG_LIBS"
#    fi
#fi
#
#AC_SUBST([WINDRES])
#AC_SUBST([IMG_LIBS])
#
#OBJCFLAGS=$CFLAGS



# Makefile.am for the SDL sample image loading library and viewer

#lib_LTLIBRARIES = libSDL2_image.la
#
#libSDL2_imageincludedir = $(includedir)/SDL2
#libSDL2_imageinclude_HEADERS =	\
#	SDL_image.h
#
#if USE_IMAGEIO
#IMAGEIO_SOURCE = IMG_ImageIO.m
#endif
#
libSDL2_image_SOURCES = [
	'IMG.c',
	'IMG_bmp.c',
	'IMG_gif.c',
	'IMG_jpg.c',
	'IMG_lbm.c',
	'IMG_pcx.c',
	'IMG_png.c',
	'IMG_pnm.c',
	'IMG_svg.c',
	'IMG_tga.c',
	'IMG_tif.c',
	'IMG_xcf.c',
	'IMG_xpm.c',
	'IMG_xv.c',
	'IMG_webp.c',
	'IMG_WIC.c',
	#	$(IMAGEIO_SOURCE)	\
	'miniz.h',
	'nanosvg.h',
	'nanosvgrast.h',
]
#
#if USE_VERSION_RC
#libSDL2_image_la_LDFLAGS = 	\
#	-no-undefined		\
#	-release $(LT_RELEASE)	\
#	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -Wl,version.o
#libSDL2_image_la_LIBADD = $(IMG_LIBS)
#libSDL2_image_la_DEPENDENCIES = version.o
#else
#libSDL2_image_la_LDFLAGS = 	\
#	-no-undefined		\
#	-release $(LT_RELEASE)	\
#	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
#libSDL2_image_la_LIBADD = $(IMG_LIBS)
#endif
#
#%.o : %.rc
#	$(WINDRES) $< $@

SDL2_image_LIBRARY = static_library('SDL2_image',libSDL2_image_SOURCES, dependencies: deps)
sdl2_image_dep = declare_dependency(link_with: SDL2_image_LIBRARY, dependencies: deps, include_directories: include_directories('.'))
