# Thin wrapper around the CMake project
project('OpenImageIO', 'c')

cmake = import('cmake')
subproj_opts = cmake.subproject_options()

subproj_opts.add_cmake_defines({
# DUSE_PTEX=0
	'BUILD_SHARED_LIBS': false,
	'CMAKE_CXX_STANDARD': 17,
	'EMBEDPLUGINS': true, # Of course, we want a single static library
	'INTERNALIZE_FMT': false, # OIIO try to copy fmt headers otherwise and fails
	'LINKSTATIC': true, # Enhance behavior
	'USE_EXTERNAL_PUGIXML': false, # No I don't want to dynlink to pugixml
	'USE_STD_FILESYSTEM': true, # We don't use boost::filesystem
	
	# Disable unused parts
	'BUILD_DOCS': false,
	'BUILD_MISSING_FMT': false,
	'BUILD_MISSING_ROBINMAP': get_option('wrap_mode') != 'nodownload', # Respect --wrap-mode=nodownload
	'BUILD_TESTING': false,
	'INSTALL_DOCS': false,
	'INSTALL_FONTS': false,
	'OIIO_BUILD_TESTS': false,
	'OIIO_BUILD_TOOLS': false,
	'OIIO_STATIC_DEFINE': true,
	
	# Setup plugins
	'ENABLE_CINEON': true,
	'ENABLE_DDS': true,
	'ENABLE_DPX': false, # Our CMake OpenEXR injection only setup Imath, not the full OpenEXR
	'ENABLE_OPENEXR': false, # Our CMake OpenEXR injection only setup Imath, not the full OpenEXR
	#'ENABLE_GIF': true,
	'ENABLE_JPEG': true,
	#'enable_png=enabled',
	'ENABLE_PSD': true,
	'ENABLE_TIFF': true,
	
	# Disable dependencies
	'USE_DCMTK': false,
	'USE_FFMPEG': false,
	'USE_FREETYPE': false,
	'USE_GIF': false, # TODO Restore
	'USE_JPEGTURBO': get_option('use_jpegturbo'),
	'USE_LIBHEIF': false,
	'USE_LIBRAW': false,
	'USE_NUKE': false,
	'USE_OPENCOLORIO': false,
	'USE_OPENCV': false,
	'USE_OPENJPEG': false,
	'USE_OPENVDB': false,
	'USE_PYTHON': false,
	'USE_PNG': false, # TODO Restore
	'USE_PTEX': false,
	'USE_R3DSDK': false,
	'USE_TBB': false,
	'USE_QT': false,
	'USE_WEBP': false,
	
	'CMAKE_FIND_PACKAGE_PREFER_CONFIG': true,
	'Boost_ROOT': get_option('cmake_overrides_path'),
	'BZip2_ROOT': get_option('cmake_overrides_path'),
	'fmt_ROOT': get_option('cmake_overrides_path'),
	'JPEG_ROOT': get_option('cmake_overrides_path'),
	'OpenEXR_ROOT': get_option('cmake_overrides_path'),
	'TIFF_ROOT': get_option('cmake_overrides_path'),
	'ZLIB_ROOT': get_option('cmake_overrides_path'),
})
subproj_opts.set_install(false)
subproj_oiio = cmake.subproject('oiio', options: subproj_opts)
oiio_dep = declare_dependency(dependencies: [
	subproj_oiio.dependency('OpenImageIO').as_link_whole(),
	subproj_oiio.dependency('OpenImageIO_Util').as_link_whole(),
	dependency('boost'),
	dependency('fmt'),
	dependency('libjpeg'),
	dependency('libtiff-4'),
	dependency('Imath'),
	dependency('zlib'),
], version: '2.4.4.2', compile_args: [
	'-DOIIO_STATIC_DEFINE',
])
meson.override_dependency('OpenImageIO',oiio_dep)
