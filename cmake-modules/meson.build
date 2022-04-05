# Force CMake to use our embeded version of libraries and other tweaks
if enable_native_progs
	cmake = import('cmake', required: false, disabler: true)
else # Skip CMake
	cmake = disabler()
endif

if cmake.found()
	cmake_module_path = ';'.join([
		meson.current_build_dir(),
		meson.current_source_dir(),
	]).replace('\\','\\\\') # Escapes backslashes
	
	dep_zlib = dependency('zlib')
	if dep_zlib.type_name() == 'internal'
		proj_zlib = subproject('zlib')
		configure_file(input: 'FindZLIB-template.cmake', output: 'FindZLIB.cmake', configuration: {
			'ZLIB_INCLUDE_DIR': proj_zlib.get_variable('zlib_source_dir'),
			'ZLIB_LIBRARY': proj_zlib.get_variable('zlib').full_path(),
			'ZLIB_VERSION_STRING': dep_zlib.version()
		})
	endif
endif
