# Thin wrapper around the CMake project
project('CURL', 'c')

cc = meson.get_compiler('c')
cmake = import('cmake')
subproj_opts = cmake.subproject_options()
subproj_opts.add_cmake_defines({
	'BUILD_CURL_EXE': false,
	'BUILD_SHARED_LIBS': false,
	'BUILD_TESTING': false,
	'CURL_USE_LIBSSH': false,
	'CURL_USE_LIBSSH2': false,
	'CURL_LTO': get_option('b_lto'),
	'CURL_ENABLE_EXPORT_TARGET': false,
	'CURL_DISABLE_COOKIES': true,
	'CURL_DISABLE_GETOPTIONS': true,
	'CURL_DISABLE_HSTS': true, # Arcollect is HTTPS-only, HSTS is useless
	'CURL_DISABLE_PROGRESS_METER': true,
	'ENABLE_DEBUG': false,
	'ENABLE_IPV6': true,
	'ENABLE_MANUAL': false,
	'ENABLE_UNIX_SOCKETS': false,
	'HTTP_ONLY': true,
	'SSL_ENABLED': true,
})
dep_curl = []
if host_machine.system() == 'windows'
	subproj_opts.add_cmake_defines({
		'CURL_USE_SCHANNEL': true,
	})
	dep_curl += cc.find_library('advapi32')
	dep_curl += cc.find_library('crypt32')
elif host_machine.system() == 'darwin'
	subproj_opts.add_cmake_defines({
		'CURL_USE_SECTRANSP': true,
	})
	dep_curl += cc.find_library('CoreFoundation')
	dep_curl += cc.find_library('Security')
endif
subproj_opts.set_install(false)
subproj_curl = cmake.subproject('libcurl-cmake', options: subproj_opts)
dep_curl_cmake = subproj_curl.dependency('libcurl')
meson.override_dependency('libcurl',declare_dependency(dependencies: [dep_curl_cmake,dep_curl], version: '7.81.0'))
