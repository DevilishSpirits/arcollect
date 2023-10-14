## Copyright 2008-present Contributors to the OpenImageIO project.
## SPDX-License-Identifier: BSD-3-Clause
## https://github.com/OpenImageIO/oiio/blob/master/LICENSE.md

imageio_pvt_h = configure_file(input: 'imageio_pvt.h.in', output: 'imageio_pvt.h', copy: true)

#if (NOT USE_EXTERNAL_PUGIXML)
#    list (APPEND libOpenImageIO_hdrs
#          ../include/OpenImageIO/detail/pugixml/pugiconfig.hpp
#          ../include/OpenImageIO/detail/pugixml/pugixml.hpp
#          ../include/OpenImageIO/detail/pugixml/pugixml.cpp
#    )
#    if (CMAKE_COMPILER_IS_GNUCC AND NOT ${GCC_VERSION} VERSION_LESS 6.0)
#        set_source_files_properties (formatspec.cpp xmp.cpp
#                                     PROPERTIES COMPILE_FLAGS -Wno-error=placement-new)
#    endif ()
#endif()

#if (CMAKE_COMPILER_IS_GNUCC AND ${GCC_VERSION} VERSION_GREATER_EQUAL 9.0)
#    set_source_files_properties (../libutil/SHA1.cpp
#                                 PROPERTIES COMPILE_FLAGS -Wno-stringop-truncation)
#endif ()

libOpenImageIO_srcs = [
	'imagebufalgo.cpp',
	'imagebufalgo_pixelmath.cpp',
	'imagebufalgo_channels.cpp',
	#'imagebufalgo_compare.cpp', Not used in Arcollect
	'imagebufalgo_copy.cpp',
	#'imagebufalgo_deep.cpp', Not used in Arcollect
	'imagebufalgo_draw.cpp',
	'imagebufalgo_addsub.cpp',
	'imagebufalgo_muldiv.cpp',
	#'imagebufalgo_mad.cpp', Not used in Arcollect
	'imagebufalgo_orient.cpp',
	'imagebufalgo_xform.cpp',
	#'imagebufalgo_yee.cpp', Not used in Arcollect
	'imagebufalgo_opencv.cpp',
	'color_ocio.cpp',
	'deepdata.cpp',
	'exif.cpp',
	'exif-canon.cpp',
	'formatspec.cpp',
	'icc.cpp',
	'imagebuf.cpp',
	'imageinput.cpp',
	'imageio.cpp',
	'imageioplugin.cpp',
	'imageoutput.cpp',
	'iptc.cpp',
	'xmp.cpp',
	#'color_ocio.cpp', Not used in Arcollect
	#'maketexture.cpp', Not used in Arcollect
	'bluenoise.cpp',
	#'../libutil/argparse.cpp', Not used in Arcollect
	#'../libutil/benchmark.cpp', Not used in Arcollect
	#'../libutil/errorhandler.cpp', Not used in Arcollect
	'../libutil/filesystem.cpp', 
	#'../libutil/farmhash.cpp', Not used in Arcollect
	'../libutil/filter.cpp', 
	'../libutil/fmath.cpp',
	#'../libutil/hashes.cpp', Not used in Arcollect
	'../libutil/paramlist.cpp', 
	'../libutil/plugin.cpp', 
	#'../libutil/SHA1.cpp', Not used in Arcollect
	'../libutil/strutil.cpp', 
	'../libutil/sysutil.cpp', 
	'../libutil/thread.cpp', 
	'../libutil/timer.cpp', 
	'../libutil/typedesc.cpp', 
	'../libutil/ustring.cpp', 
	#'../libutil/xxhash.cpp', Not used in Arcollect
	'../libtexture/texturesys.cpp', 
	'../libtexture/texture3d.cpp', 
	'../libtexture/environment.cpp', 
	'../libtexture/texoptions.cpp', 
	'../libtexture/imagecache.cpp',
	#${libOpenImageIO_srcs},
	#${libOpenImageIO_hdrs},
	#'../include/OpenImageIO/detail/pugixml/pugiconfig.hpp', Not used in Arcollect
	#'../include/OpenImageIO/detail/pugixml/pugixml.hpp', Not used in Arcollect
	#'../include/OpenImageIO/detail/pugixml/pugixml.cpp', Not used in Arcollect
	imageio_pvt_h,
	'../include/OpenImageIO/argparse.h',
	'../include/OpenImageIO/array_view.h',
	'../include/OpenImageIO/atomic.h',
	'../include/OpenImageIO/attrdelegate.h',
	'../include/OpenImageIO/benchmark.h',
	'../include/OpenImageIO/color.h',
	'../include/OpenImageIO/dassert.h',
	'../include/OpenImageIO/deepdata.h',
	'../include/OpenImageIO/errorhandler.h',
	'../include/OpenImageIO/export.h',
	'../include/OpenImageIO/filesystem.h',
	'../include/OpenImageIO/filter.h',
	'../include/OpenImageIO/fmath.h',
	'../include/OpenImageIO/fstream_mingw.h',
	'../include/OpenImageIO/function_view.h',
	'../include/OpenImageIO/hash.h',
	'../include/OpenImageIO/imagebuf.h',
	'../include/OpenImageIO/imagebufalgo.h',
	'../include/OpenImageIO/imagebufalgo_util.h',
	'../include/OpenImageIO/imagecache.h',
	'../include/OpenImageIO/imageio.h',
	'../include/OpenImageIO/image_view.h',
	Imath_h,
	half_h,
	'../include/OpenImageIO/missing_math.h',
	oiioversion_h,
	'../include/OpenImageIO/optparser.h',
	'../include/OpenImageIO/parallel.h',
	'../include/OpenImageIO/paramlist.h',
	'../include/OpenImageIO/platform.h',
	'../include/OpenImageIO/plugin.h',
	'../include/OpenImageIO/refcnt.h',
	'../include/OpenImageIO/simd.h',
	'../include/OpenImageIO/span.h',
	'../include/OpenImageIO/strided_ptr.h',
	'../include/OpenImageIO/string_view.h',
	'../include/OpenImageIO/strutil.h',
	'../include/OpenImageIO/sysutil.h',
	'../include/OpenImageIO/texture.h',
	'../include/OpenImageIO/thread.h',
	'../include/OpenImageIO/tiffutils.h',
	'../include/OpenImageIO/timer.h',
	'../include/OpenImageIO/typedesc.h',
	'../include/OpenImageIO/unittest.h',
	'../include/OpenImageIO/unordered_map_concurrent.h',
	'../include/OpenImageIO/ustring.h',
	'../include/OpenImageIO/varyingref.h',
	'../include/OpenImageIO/version.h',
]

OpenImageIO_Util_deps = [
	oiio_plugins,
	oiio_deps,
	cpp.find_library('dl', required: false),
]
OpenImageIO = static_library('OpenImageIO', libOpenImageIO_srcs, dependencies: OpenImageIO_Util_deps, include_directories: include_directories('../include'), cpp_args: compile_args_priv, build_by_default: false)
