project('arcollect', 'cpp',
	# x.y Version numbering
	# | |
	# | +-- Build: Incremented to rebuild webextension and public packages.
	# |
	# +---- Minor: Will be incremented one day
	version: '0.15',
	meson_version: '>=0.59.0',
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
else
	with_xdg = with_xdg.enabled()
endif

# Basic configuration
config_h = configuration_data()
config_h.set('ARCOLLECT_VERSION',meson.project_version())
config_h.set('ARCOLLECT_WEBSITE','https://github.com/DevilishSpirits/arcollect')
config_h.set('ARCOLLECT_MOZILLA_WEBEXT_ID','arcollect@d-spirits.me')
config_h.set('HAS_SDL_OPENURL','1') # TODO Probe that
config_h.set('WITH_XDG',with_xdg ? '1' : '0')
config_h.set('CPP_STD',get_option('cpp_std').to_upper())
if (cpp.get_id() == 'gcc')
	config_h.set('CXX_COMPILER_TITLE','GCC '+cpp.version())
elif (cpp.get_id() == 'clang')or(cpp.get_id() == 'clang-cl')
	config_h.set('CXX_COMPILER_TITLE','LLVM Clang '+cpp.version())
elif (cpp.get_id() == 'msvc')
	config_h.set('CXX_COMPILER_TITLE','Microsoft Visual Studio '+cpp.version())
else
	config_h.set(cpp.get_id()+' C++ compiler '+cpp.version())
endif

# D-Bus stuff
config_h.set('ARCOLLECT_DBUS_NAME','me.d-spirits.arcollect')
config_h.set('ARCOLLECT_DBUS_PATH','/')

# Version requirements
# These are centralized here. Update README.md if you change things there.
#config_h.set('ARCOLLECT_VERREQ_sdl2','>=2.0.x') # TODO Switch back to an SDL release when the ICC profile patch get released
config_h.set('ARCOLLECT_VERREQ_sqlite3','>=3.35.0')

foreach key: config_h.keys()
	config_h.set_quoted(key+'_STR',config_h.get(key))
	summary(key,config_h.get(key), section: 'config.h')
endforeach

configure_file(output: 'config.h', configuration: config_h)
config_h_inc = include_directories('.')

# Dependencies
if enable_native_progs
	dep_curl = dependency('libcurl', required: true)
	
	# SDL_GetWindowICCProfile has not been released yet
	#dep_sdl2 = dependency('sdl2', version: config_h.get('ARCOLLECT_VERREQ_sdl2'))
	dep_sdl2 = subproject('sdl2').get_variable('sdl2_dep')
	
	dep_oiio = dependency('OpenImageIO', fallback: ['oiio', 'dep_oiio'], default_options: [
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
	dep_freetype   = dependency('freetype2')
	dep_harfbuzz   = dependency('harfbuzz')
	dep_sqlite3 = dependency('sqlite3', version: config_h.get('ARCOLLECT_VERREQ_sqlite3'))
	dep_inih = dependency('INIReader', fallback: ['inih', 'INIReader_dep'], default_options: [
		'with_INIReader=true',
		'distro_install=false',
	])
	dep_rapidjson = dependency('rapidjson')
endif

# Track dependencies
# It's important to clearly state what is embeded
system_dependencies = []
static_dependencies = []
if enable_native_progs
	# TODO Add sdl2 when no longer using a special branch
	native_progs_deps = [
		'freetype2',
		'harfbuzz',
		'lcms2',
		'libcurl',
		'OpenImageIO',
		'rapidjson',
		'sdl2',
		'sqlite3',
	]
	if with_xdg
		native_progs_deps += 'dbus-1'
	endif
	foreach dep_name: native_progs_deps
		dep = dependency(dep_name, required: false)
		if dep.found()
			if dep.type_name() == 'internal'
				static_dependencies += dep_name
			else
				system_dependencies += dep_name
			endif
		endif
	endforeach
endif

# TODO if 'OpenImageIO' in static_dependencies
#endif

if (host_machine.system() == 'windows') and (system_dependencies.length() > 0)
	deps = static_dependencies+system_dependencies
	warning('You are linking to DLLs!. You should reconfigure with --force-fallback-for='+','.join(deps))
endif

foreach dep: system_dependencies
	summary(dep,true , section: 'Using system dependencies (NO if embeded)', bool_yn: true)
endforeach
foreach dep: static_dependencies
	summary(dep,false, section: 'Using system dependencies (NO if embeded)', bool_yn: true)
endforeach

# Generate dependencies informations
dep_fancy_name = {
	'base64'           : 'Base64.h',
	'dbus-1'           : 'D-Bus',
	'freetype2'        : 'FreeType2',
	'harfbuzz'         : 'HarfBuzz',
	'inih'             : 'INIReader',
	'lcms2'            : 'LittleCMS',
	'libcurl'             : 'curl',
	'OpenImageIO'      : 'OpenImageIO',
	'rapidjson'        : 'RapidJSON',
	'roboto'           : 'Roboto',
	'sdl2'             : 'SDL',
	'sqlite3'          : 'SQLite',
}
dep_website = {
	'base64'           : 'https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594',
	'dbus-1'           : 'https://www.freedesktop.org/wiki/Software/dbus/',
	'freetype2'        : 'https://www.freetype.org',
	'harfbuzz'         : 'https://harfbuzz.github.io/',
	'inih'             : 'https://github.com/benhoyt/inih',
	'lcms2'            : 'https://littlecms.com/',
	'libcurl'             : 'https://curl.se/',
	'OpenImageIO'      : 'https://openimageio.readthedocs.org/',
	'rapidjson'        : 'https://rapidjson.org/',
	'roboto'           : 'https://fonts.google.com/specimen/Roboto',
	'sdl2'             : 'https://www.libsdl.org/',
	'sqlite3'          : 'https://www.sqlite.org/',
}

# TODO Get version of these
also_embeded = [
	'base64',
	'roboto',
]

if (static_dependencies.length() + also_embeded.length() > 0)
	about_embeded_dependencies = []
	last_embeded_dependencies = ''
	foreach dep: static_dependencies
		if last_embeded_dependencies != ''
			about_embeded_dependencies += last_embeded_dependencies
		endif
		last_embeded_dependencies = dep_fancy_name[dep]+' '+dependency(dep).version()+' ('+dep_website[dep]+')'
	endforeach
	foreach dep: also_embeded
		if last_embeded_dependencies != ''
			about_embeded_dependencies += last_embeded_dependencies
		endif
		last_embeded_dependencies = dep_fancy_name[dep]+' ('+dep_website[dep]+')'
	endforeach
	if ('OpenImageIO' in static_dependencies)
		if last_embeded_dependencies != ''
			about_embeded_dependencies += last_embeded_dependencies
		endif
		last_embeded_dependencies = 'TODO: Dependencies of OpenImageIO (checkout the source code for now)'
	endif
	about_embeded_dependencies = ', '.join(about_embeded_dependencies) + ' and ' + last_embeded_dependencies
	message('This copy of Arcollect will embed copies of '+about_embeded_dependencies)
endif

dependency_report_data = configuration_data()
dependency_report_data.set_quoted('ABOUT_EMBEDED_DEPENDENCIES_STR',about_embeded_dependencies.replace('\n','\\n'))
dependency_report_hpp = configure_file(output: 'dependency-report.hpp', configuration: dependency_report_data)

# Add subdir (include order is important !)
if enable_native_progs
	subdir('sqls')
	subdir('common')
	subdir('desktop-app')
	subdir('webext-adder')
endif
if enable_webextension
	subdir('webextension')
endif
subdir('packaging')
