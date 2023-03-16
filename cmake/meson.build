# Force CMake to use our embeded version of libraries and other tweaks
if enable_native_progs
	cmake = import('cmake', required: false, disabler: true)
else # Skip CMake
	cmake = disabler()
endif

if cmake.found()
	cmake_overrides_path = meson.current_build_dir().replace('\\','\\\\') # Escapes backslashes
	
	cmake_overrides_defines = {
		'CMAKE_FIND_USE_PACKAGE_ROOT_PATH': true, # Forcibly tell CMake to read <Package>_FOUND
		'CMAKE_FIND_PACKAGE_PREFER_CONFIG': true, # Search for config files before Find<Package>
	}
	
	dep_zlib = dependency('zlib')
	if dep_zlib.type_name() == 'internal'
		proj_zlib = subproject('zlib')
		zlib_conf = {
			'ZLIB_INCLUDE_DIR': proj_zlib.get_variable('zlib_source_dir').replace('\\','\\\\'),
			'ZLIB_LIBRARY': proj_zlib.get_variable('zlib').full_path().replace('\\','\\\\'),
			'ZLIB_VERSION_STRING': dep_zlib.version()
		}
		configure_file(input: 'ZLIBConfig.cmake', output: 'ZLIBConfig.cmake', configuration: zlib_conf)
		cmake_overrides_defines += {'ZLIB_ROOT':cmake_overrides_path}
	endif
	
	dep_boost = dependency('boost')
	if dep_boost.type_name() == 'internal'
		proj_boost = subproject('boost')
		boost_conf = {
			'BOOST_INCLUDE_DIR': proj_boost.get_variable('boost_incdir').replace('\\','\\\\'),
			'BOOST_VERSION_STRING': dep_boost.version(),
		}
		configure_file(input: 'BoostConfig.cmake', output: 'BoostConfig.cmake', configuration: boost_conf)
		cmake_overrides_defines += {'Boost_ROOT':cmake_overrides_path}
	endif
	
	dep_brotli = dependency('libbrotlicommon')
	if dep_brotli.type_name() == 'internal'
		proj_brotli = subproject('brotli')
		brotli_conf = {
			'BROTLI_INCLUDE_DIR': proj_brotli.get_variable('cmake_brotli_include_dir').replace('\\','\\\\'),
			'BROTLICOMMON_LIBRARY': proj_brotli.get_variable('libbrotli_common').full_path().replace('\\','\\\\'),
			'BROTLIDEC_LIBRARY': proj_brotli.get_variable('libbrotli_decoder').full_path().replace('\\','\\\\'),
			'BROTLIENC_LIBRARY': proj_brotli.get_variable('libbrotli_encoder').full_path().replace('\\','\\\\'),
		}
		configure_file(input: 'BrotliConfig.cmake', output: 'BrotliConfig.cmake', configuration: brotli_conf)
		cmake_overrides_defines += {'Brotli_ROOT':cmake_overrides_path}
	endif
	
	dep_Imath = dependency('Imath')
	if dep_Imath.type_name() == 'internal'
		proj_Imath = subproject('Imath')
		Imath_conf = {
			'ILMBASE_LIBRARIES': proj_Imath.get_variable('Imath').full_path().replace('\\','\\\\'),
			'IMATH_INCLUDES': proj_Imath.get_variable('Imath_incdir').replace('\\','\\\\'),
			'IMATH_VERSION_STRING': dep_Imath.version(),
		}
		configure_file(input: 'OpenEXRConfig.cmake', output: 'OpenEXRConfig.cmake', configuration: Imath_conf)
		cmake_overrides_defines += {'OpenEXR_ROOT':cmake_overrides_path}
	endif
	
	dep_fmt = dependency('fmt')
	if dep_fmt.type_name() == 'internal'
		proj_fmt = subproject('fmt')
		fmt_conf = {
			'FMT_INCLUDE_DIR': proj_fmt.get_variable('fmt_incdir').replace('\\','\\\\'),
			'FMT_VERSION_STRING': dep_fmt.version(),
		}
		configure_file(input: 'fmtConfig.cmake', output: 'fmtConfig.cmake', configuration: fmt_conf)
		cmake_overrides_defines += {'fmt_ROOT':cmake_overrides_path}
	endif
	
	dep_jpeg = dependency('libjpeg')
	if dep_jpeg.type_name() == 'internal'
		proj_jpeg = subproject('libjpeg')
		jpeg_conf = {
			'JPEG_INCLUDE_DIR': proj_jpeg.get_variable('jpeg_incdir').replace('\\','\\\\'),
			'JPEG_INCLUDE_DIR2': proj_jpeg.get_variable('jpeg_incdir2').replace('\\','\\\\'),
			'JPEG_LIBRARY': proj_jpeg.get_variable('jpeg').full_path().replace('\\','\\\\'),
			'JPEG_VERSION_STRING': dep_jpeg.version(),
		}
		configure_file(input: 'JPEGConfig.cmake', output: 'JPEGConfig.cmake', configuration: jpeg_conf+{'JPEG_PREFIX': 'JPEG'})
		cmake_overrides_defines += {'JPEG_ROOT':cmake_overrides_path}
	endif
	
	dep_tiff = dependency('libtiff-4')
	if dep_tiff.type_name() == 'internal'
		proj_tiff = subproject('libtiff')
		tiff_conf = {
			'TIFF_INCLUDE_DIR': proj_tiff.get_variable('libtiff4_incdir').replace('\\','\\\\'),
			'TIFF_INCLUDE_DIR2': proj_tiff.get_variable('libtiff4_incdir2').replace('\\','\\\\'),
			'TIFF_LIBRARY': proj_tiff.get_variable('tiff4_lib').full_path().replace('\\','\\\\'),
			'TIFF_VERSION_STRING': dep_tiff.version(),
		}
		configure_file(input: 'TIFFConfig.cmake', output: 'TIFFConfig.cmake', configuration: tiff_conf)
		cmake_overrides_defines += {'TIFF_ROOT':cmake_overrides_path}
	endif
endif
