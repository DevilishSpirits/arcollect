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
	dep_curl = dependency('libcurl', required: true)
	
	# SDL_GetWindowICCProfile has not been released yet
	#dep_sdl2 = dependency('sdl2', version: config_h.get('ARCOLLECT_VERREQ_sdl2'))
	dep_sdl2 = subproject('sdl2').get_variable('sdl2_dep')
	
	dep_oiio = dependency('OpenImageIO', default_options: [
		'enable_bmp=enabled',
		'enable_cineon=enabled',
		'enable_dds=enabled',
		'enable_dpx=auto', # TODO OpenEXR wrap
		'enable_gif=enabled',
		'enable_jpeg=enabled',
		'enable_jpeg2000=disabled', # FIXME It's broken
		'enable_png=enabled',
		'enable_psd=enabled',
		'enable_tiff=enabled',
	])
	
	dep_lcms2  = dependency('lcms2')
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
