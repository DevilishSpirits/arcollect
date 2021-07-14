project('arcollect', 'cpp',
	# x.y Version numbering
	# | |
	# | +-- Build: Incremented to rebuild webextension and public packages.
	# |
	# +---- Minor: Will be incremented one day
	version: '0.10',
	meson_version: '>=0.57.1',
	default_options: [
		'cpp_std=c++17',
		#'cpp_rtti=false',
		'default_library=static', # For subprojects
	])

cpp = meson.get_compiler('cpp')

# Options
enable_webextension = get_option('enable_webextension')
enable_native_progs = get_option('enable_native_progs')
with_xdg = get_option('with_xdg')

# Detect environment
dep_dbus = dependency('dbus-1', required: with_xdg)
if with_xdg.auto()
	with_xdg = false
	if host_machine.system() in [
		'android',
		'cygwin',
		'darwin',
		'emscripten',
		'windows',
	]
		# With are not on a XDG platform
	elif not cpp.has_header('spawn.h')
		warning('XDG integration disabled due to missing <spawn.h>')
	elif not cpp.has_function('posix_spawnp', prefix:'#include <spawn.h>')
		warning('XDG integration disabled due to missing posix_spawnp')
	elif not cpp.links('extern char** environ;int main(void){return environ != nullptr;}')
		warning('XDG integration disabled due to missing "extern char** environ;"')
	elif not dep_dbus.found()
		warning('XDG integration disabled due to missing D-Bus library')
	else # XDG test passed
		with_xdg = true
	endif
elif
	with_xdg = with_xdg.enabled()
endif

# Check for threading support
with_mingw_std_threads = false
if enable_native_progs
	# Check if threading is supported
	threading_tests = {
		'std::mutex': 'mutex',
		'std::thread': 'thread',
		'std::lock_guard': 'mutex',
		'std::condition_variable': 'condition_variable',
	}
	has_threading = true
	foreach type, header: threading_tests
		if not cpp.has_header_symbol(header,type)
			has_threading = false
		endif
	endforeach
	# If not, try to use a third-party library
	if not has_threading
		if (host_machine.system() == 'windows') and (meson.get_compiler('cpp').get_id() == 'gcc')
			# Use MinGW std threads on MinGW
			with_mingw_std_threads = true
		else
			# fail
			error('Threading support is required.')
		endif
	endif
endif

# Basic configuration
config_h = configuration_data()
config_h.set('ARCOLLECT_VERSION',meson.project_version())
config_h.set('ARCOLLECT_WEBSITE','https://github.com/DevilishSpirits/arcollect')
config_h.set('ARCOLLECT_MOZILLA_WEBEXT_ID','arcollect@d-spirits.me')
config_h.set('WITH_XDG',with_xdg ? '1' : '0')
config_h.set('WITH_MINGW_STD_THREADS',with_mingw_std_threads ? '1' : '0')

# D-Bus stuff
config_h.set('ARCOLLECT_DBUS_NAME','me.d-spirits.arcollect')
config_h.set('ARCOLLECT_DBUS_PATH','/')

# Version requirements
# These are centralized here. Update README.md if you change things there.
config_h.set('ARCOLLECT_VERREQ_sdl2','>=2.0.8')
config_h.set('ARCOLLECT_VERREQ_sdl2_ttf','>=2.0.12')
config_h.set('ARCOLLECT_VERREQ_sqlite3','>=3.35.0')

foreach key: config_h.keys()
	config_h.set_quoted(key+'_STR',config_h.get(key))
	summary(key,config_h.get(key), section: 'config.h')
endforeach

configure_file(output: 'config.h', configuration: config_h)
config_h_inc = include_directories('.')

# Programs
prog_zip = find_program('zip', native: true, required: enable_webextension)
# Dependencies
if enable_native_progs
	# Hack to have --force-fallback-for libcurl works
	dep_curl = dependency('libcurl', required: false, fallback: ['not-buildable','dummy'])
	if not dep_curl.found()
		cmake = import('cmake')
		subproj_opts_curl = cmake.subproject_options()
		subproj_opts_curl.add_cmake_defines({
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
			subproj_opts_curl.add_cmake_defines({
				'CMAKE_USE_SCHANNEL': true,
			})
			dep_curl += cpp.find_library('advapi32')
			dep_curl += cpp.find_library('crypt32')
		elif host_machine.system() == 'darwin'
			subproj_opts_curl.add_cmake_defines({
				'CMAKE_USE_SECTRANSP': true,
			})
			dep_curl += cpp.find_library('CoreFoundation')
			dep_curl += cpp.find_library('Security')
		endif
		subproj_opts_curl.set_install(false)
		subproj_curl = cmake.subproject('libcurl', options: subproj_opts_curl)
		dep_curl_cmake = subproj_curl.dependency('libcurl')
		dep_curl = declare_dependency(dependencies: [dep_curl_cmake,dep_curl], version: dep_curl_cmake.version())
	endif
	
	if (host_machine.system() == 'windows') and (meson.get_compiler('c').get_id() == 'gcc')
		# Use MinGW prebuilt SDL2 on Windows platform (fix wrap build on GitHub Actions Windows 2019)
		dep_sdl2 = dependency('sdl2', fallback: ['sdl2-mingw', 'sdl2_dep'])
	else # Normal behavior
		dep_sdl2 = dependency('sdl2', version: config_h.get('ARCOLLECT_VERREQ_sdl2'))
	endif
	dep_oiio = dependency('OpenImageIO')
	
	dep_freetype2  = dependency('freetype2')
	dep_sdl2_ttf   = dependency('SDL2_ttf', version: config_h.get('ARCOLLECT_VERREQ_sdl2_ttf'))
	dep_sqlite3 = dependency('sqlite3', version: config_h.get('ARCOLLECT_VERREQ_sqlite3'))
	dep_inih = dependency('INIReader', fallback: ['inih', 'INIReader_dep'], default_options: [
		'with_INIReader=true',
		'distro_install=false',
	])
	dep_rapidjson = dependency('rapidjson')
endif

# Add subdir (include order is important !)
if enable_native_progs
	subdir('db-schema')
	subdir('common')
	subdir('desktop-app')
	subdir('webext-adder')
endif
if enable_webextension
	subdir('webextension')
endif
subdir('packaging')
