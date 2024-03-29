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
endif
