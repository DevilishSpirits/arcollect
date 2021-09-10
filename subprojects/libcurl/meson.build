# Thin wrapper around the CMake project
project('CURL', 'c', version: '7.78.0')

cc = meson.get_compiler('c')
cmake = import('cmake')
subproj_opts = cmake.subproject_options()
subproj_opts.add_cmake_defines({
	'BUILD_CURL_EXE': false,
	'BUILD_SHARED_LIBS': false,
	'BUILD_TESTING': false,
	'CMAKE_USE_LIBSSH': false,
	'CMAKE_USE_LIBSSH2': false,
	'CURL_DISABLE_COOKIES': true,
	'CURL_DISABLE_CRYPTO_AUTH': true,
	'ENABLE_IPV6': true,
	'ENABLE_MANUAL': false,
	'ENABLE_UNIX_SOCKETS': false,
	'HTTP_ONLY': true,
	'SSL_ENABLED': true,
})
dep_curl = []
if host_machine.system() == 'windows'
	subproj_opts.add_cmake_defines({
		'CMAKE_USE_SCHANNEL': true,
	})
	dep_curl += cc.find_library('advapi32')
	dep_curl += cc.find_library('crypt32')
elif host_machine.system() == 'darwin'
	subproj_opts.add_cmake_defines({
		'CMAKE_USE_SECTRANSP': true,
	})
	dep_curl += cc.find_library('CoreFoundation')
	dep_curl += cc.find_library('Security')
endif
subproj_opts.set_install(false)
subproj_curl = cmake.subproject('libcurl-cmake', options: subproj_opts)
dep_curl_cmake = subproj_curl.dependency('libcurl')
meson.override_dependency('libcurl',declare_dependency(dependencies: [dep_curl_cmake,dep_curl], version: meson.project_version()))
