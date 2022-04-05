# Force CMake to use our embeded version of libraries and other tweaks
if enable_native_progs
	cmake = import('cmake', required: false, disabler: true)
else # Skip CMake
	cmake = disabler()
endif

if cmake.found()
	cmake_module_path = meson.current_build_dir().replace('\\','\\\\') # Escapes backslashes
	
	dep_zlib = dependency('zlib')
	if dep_zlib.type_name() == 'internal'
		proj_zlib = subproject('zlib')
		configure_file(input: 'FindZLIB.cmake', output: 'FindZLIB.cmake', configuration: {
			'ZLIB_INCLUDE_DIR': proj_zlib.get_variable('zlib_source_dir').replace('\\','\\\\'),
			'ZLIB_LIBRARY': proj_zlib.get_variable('zlib').full_path().replace('\\','\\\\'),
			'ZLIB_VERSION_STRING': dep_zlib.version()
		})
	endif
	
	dep_brotli = dependency('libbrotlicommon')
	if dep_brotli.type_name() == 'internal'
		proj_brotli = subproject('brotli')
		configure_file(input: 'FindBrotli.cmake', output: 'FindBrotli.cmake', configuration: {
			'BROTLI_INCLUDE_DIR': proj_brotli.get_variable('cmake_brotli_include_dir').replace('\\','\\\\'),
			'BROTLICOMMON_LIBRARY': proj_brotli.get_variable('libbrotli_common').full_path().replace('\\','\\\\'),
			'BROTLIDEC_LIBRARY': proj_brotli.get_variable('libbrotli_decoder').full_path().replace('\\','\\\\'),
			'BROTLIENC_LIBRARY': proj_brotli.get_variable('libbrotli_encoder').full_path().replace('\\','\\\\'),
		})
	else
		configure_file(input: 'FindBrotli-curl.cmake', output: 'FindBrotli.cmake', copy: true)
	endif
endif
