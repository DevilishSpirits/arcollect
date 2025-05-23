# Thin wrapper around the CMake project
project('CURL', 'c')

cc = meson.get_compiler('c')
cmake = import('cmake')
subproj_opts = cmake.subproject_options()
subproj_opts.add_cmake_defines({
	'BUILD_CURL_EXE': false,
	'BUILD_SHARED_LIBS': false,
	'BUILD_TESTING': false,
	'CURL_BROTLI': false, # TODO Restore
	'USE_LIBIDN2': false, # No supported platform use IDN currently
	'USE_MANUAL': false, # Disable doc generation
	'CURL_USE_LIBSSH': false,
	'CURL_USE_LIBSSH2': false,
	'CURL_LTO': get_option('b_lto'),
	'CURL_ENABLE_EXPORT_TARGET': false,
	'CURL_DISABLE_ALTSVC': true, # Disabled by default and not used
	'CURL_DISABLE_COOKIES': true,
	'CURL_DISABLE_GETOPTIONS': true,
	'CURL_DISABLE_HSTS': true, # Arcollect is HTTPS-only, HSTS is useless
	'CURL_DISABLE_NETRC': true, # Disabled by default and not used
	'CURL_DISABLE_PROGRESS_METER': true,
	'ENABLE_DEBUG': false,
	'ENABLE_IPV6': true,
	'ENABLE_MANUAL': false,
	'ENABLE_UNIX_SOCKETS': false,
	'HTTP_ONLY': true,
	'SSL_ENABLED': true,
	
	'CMAKE_FIND_PACKAGE_PREFER_CONFIG': true,
	'ZLIB_ROOT': get_option('cmake_overrides_path'),
	'Brotli_ROOT': get_option('cmake_overrides_path'),
})
dep_curl = [dependency('zlib'),dependency('libbrotlidec')]
if host_machine.system() == 'windows'
	subproj_opts.add_cmake_defines({
		'CURL_USE_SCHANNEL': true,
	})
	dep_curl += cc.find_library('advapi32')
	dep_curl += cc.find_library('crypt32')
	dep_curl += cc.find_library('Bcrypt')
elif host_machine.system() == 'darwin'
	subproj_opts.add_cmake_defines({
		'CURL_USE_SECTRANSP': true,
	})
	dep_curl += cc.find_library('CoreFoundation')
	dep_curl += cc.find_library('Security')
endif
subproj_opts.set_install(false)
subproj_curl = cmake.subproject('libcurl-cmake', options: subproj_opts)
curl_version = cc.get_define('LIBCURL_VERSION', include_directories: subproj_curl.include_directories('libcurl_static'), prefix: '#include <curl/curl.h>').strip('"')
meson.override_dependency('libcurl',declare_dependency(dependencies: [subproj_curl.dependency('libcurl_static'),dep_curl], version: curl_version))
