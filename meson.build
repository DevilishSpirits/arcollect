project('arcollect', 'cpp',
	# x.y Version numbering
	# | |
	# | +-- Build: Incremented to rebuild webextension and public packages.
	# |
	# +---- Minor: Will be incremented one day
	version: '0.23',
	meson_version: '>=0.59.0',
	default_options: [
		'cpp_std=c++20',
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
		message('If you are using Linux, *BSD or alike, you likely wants XDG integration (desktop icon, ...).')
		message('The blacklist based detection *will* output false positive on uncommon systems.')
		error('XDG integration require D-Bus (you can disable it with -Dwith_xdg=disabled)')
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
config_h.set('ARCOLLECT_SUMMARY','Your personal visual artwork library.')
config_h.set('ARCOLLECT_MOZILLA_WEBEXT_ID','arcollect@d-spirits.me')
config_h.set('HAS_SDL_OPENURL','1') # TODO Probe that
config_h.set('WITH_XDG',with_xdg ? '1' : '0')
config_h.set('ARCOLLECT_X11_WM_CLASS','arcollect-desktop-app')
config_h.set('CPP_STD',get_option('cpp_std').to_upper())
if (cpp.get_id() == 'gcc')
	config_h.set('CXX_COMPILER_TITLE','GCC '+cpp.version())
elif (cpp.get_id() == 'clang')or(cpp.get_id() == 'clang-cl')
	config_h.set('CXX_COMPILER_TITLE','LLVM Clang '+cpp.version())
elif (cpp.get_id() == 'msvc')
	config_h.set('CXX_COMPILER_TITLE','MSVC '+cpp.version())
else
	config_h.set('CXX_COMPILER_TITLE',cpp.get_id()+' C++ compiler '+cpp.version())
endif

# D-Bus stuff
config_h.set('ARCOLLECT_DBUS_NAME','me.d_spirits.arcollect')
config_h.set('ARCOLLECT_DBUS_PATH','/')

# Version requirements
# These are centralized here. Update README.md if you change things there.
config_h.set('ARCOLLECT_VERREQ_sdl2','>=2.0.18')
config_h.set('ARCOLLECT_VERREQ_boost','>=1.66') # Keep in sync with OIIO needs!
config_h.set('ARCOLLECT_VERREQ_libcurl','>=7.60')
config_h.set('ARCOLLECT_VERREQ_sqlite3','>=3.35.0')

foreach key: config_h.keys()
	config_h.set_quoted(key+'_STR',config_h.get(key))
	summary(key,config_h.get(key), section: 'config.h')
endforeach

configure_file(output: 'config.h', configuration: config_h)
config_h_inc = include_directories('.')

# Dependencies
if enable_native_progs
	# Query C++ features
	cpp_required_features = {
		# C++20 features
		# C++17 features
		'__cpp_fold_expressions'       : '>=201603',
		'__cpp_lib_filesystem'         : '>=201703',
		'__cpp_lib_optional'           : '>=201606',
		'__cpp_lib_variant'            : '>=201606',
		#'__cpp_lib_to_chars'           : '>=201611', # FIXME Almost nobody implement it
		'__cpp_lib_string_view'        : '>=201606',
		# C++14 features
		'__cpp_lib_string_udls'        : '>=201304',
		'__cpp_lib_make_unique'        : '>=201304',
		# C++11 features
		'__cpp_unicode_characters'     : '>=200704',
		'__cpp_unicode_literals'       : '>=200710',
		'__cpp_lambdas'                : '>=200907',
		'__cpp_constexpr'              : '>=200704',
		'__cpp_range_based_for'        : '>=200907',
		'__cpp_static_assert'          : '>=200410',
		'__cpp_decltype'               : '>=200707',
		'__cpp_variadic_templates'     : '>=200704',
		'__cpp_initializer_lists'      : '>=200806',
		'__cpp_delegating_constructors': '>=200604',
		'__cpp_alias_templates'        : '>=200704',
		# C++98 features
		'__cpp_exceptions'             : '>=199711',
	}
	errmsg = []
	foreach feature, version_min: cpp_required_features
		version = cpp.get_define(feature, prefix: '#include <version>')
		if version == ''
			errmsg += feature+' (require '+version_min+')'
		elif not version.version_compare(version_min)
			errmsg += feature+' (found '+version+', requires '+version_min+')'
		endif
	endforeach
	if errmsg.length() > 0
		if get_option('fatal_cpp_feature_miss')
			message('You may try again with -Dfatal_cpp_feature_miss=false to bypass this check')
			error('The following C++ features are missing:',', '.join(errmsg))
		else
			warning('The following C++ features are missing:',', '.join(errmsg))
		endif
	endif
	# Query dependencies
	dep_curl = dependency('libcurl', required: true)
	
	dep_sdl2 = dependency('sdl2', version: config_h.get('ARCOLLECT_VERREQ_sdl2'))
	
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
endif

# Track dependencies
# It's important to clearly state what is embeded
system_dependencies = {}
static_dependencies = {}
if enable_native_progs
	native_progs_deps = [
		'freetype2',
		'harfbuzz',
		'lcms2',
		'libcurl',
		'OpenImageIO',
		'sdl2',
		'sqlite3',
	]
	if with_xdg
		native_progs_deps += 'dbus-1'
	endif
	if dependency('OpenImageIO', required: false).type_name() == 'internal'
		native_progs_deps += [
			'boost',
			'FMT',
			'Imath',
			'libtiff-4',
			'robin-map',
			'libjpeg',
			'libpng',
		]
	endif
	if ('libpng' in native_progs_deps) and (dependency('libpng', required: false).type_name() == 'internal')
		native_progs_deps += [
			'zlib',
		]
	endif
	foreach dep_name: native_progs_deps
		dep = dependency(dep_name, required: false)
		if dep.found()
			if dep.type_name() == 'internal'
				static_dependencies += {dep_name: dep.version()}
			else
				system_dependencies += {dep_name: dep.version()}
			endif
		endif
	endforeach
endif

if (host_machine.system() == 'windows') and (system_dependencies.keys().length() > 0)
	warning('You are linking to DLLs!. You should reconfigure with --force-fallback-for='+','.join(static_dependencies.keys()+system_dependencies.keys()))
endif

foreach dep, version: system_dependencies
	summary(dep,true , section: 'Using system dependencies (NO if embeded)', bool_yn: true)
endforeach
foreach dep, version: static_dependencies
	summary(dep,false, section: 'Using system dependencies (NO if embeded)', bool_yn: true)
endforeach

# Generate dependencies informations
dep_fancy_name = {
	'base64'           : 'Base64.h',
	'boost'            : 'Boost',
	'dbus-1'           : 'D-Bus',
	'freetype2'        : 'FreeType2',
	'FMT'              : '{fmt}',
	'harfbuzz'         : 'HarfBuzz',
	'Imath'            : 'Imath',
	'inih'             : 'INIReader',
	'lcms2'            : 'LittleCMS',
	'libcurl'          : 'curl',
	'libjpeg'          : 'libjpeg-turbo',
	'libpng'           : 'libpng',
	'libtiff-4'        : 'LibTIFF',
	'OpenImageIO'      : 'OpenImageIO',
	'robin-map'        : 'robin-ma',
	'roboto'           : 'Roboto',
	'sdl2'             : 'SDL',
	'sqlite3'          : 'SQLite',
	'zlib'             : 'zlib',
}
dep_website = {
	'base64'           : 'https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594',
	'boost'            : 'https://www.boost.org/',
	'dbus-1'           : 'https://www.freedesktop.org/wiki/Software/dbus/',
	'freetype2'        : 'https://www.freetype.org',
	'FMT'              : 'https://fmt.dev/latest/index.html',
	'harfbuzz'         : 'https://harfbuzz.github.io/',
	'Imath'            : 'https://github.com/AcademySoftwareFoundation/Imath',
	'inih'             : 'https://github.com/benhoyt/inih',
	'lcms2'            : 'https://littlecms.com/',
	'libcurl'          : 'https://curl.se/',
	'libjpeg'          : 'https://libjpeg-turbo.org/',
	'libpng'           : 'http://www.libpng.org/pub/png/libpng.html',
	'libtiff-4'        : 'http://www.simplesystems.org/libtiff/',
	'OpenImageIO'      : 'https://openimageio.org',
	'robin-map'        : 'https://github.com/Tessil/robin-map',
	'roboto'           : 'https://fonts.google.com/specimen/Roboto',
	'sdl2'             : 'https://www.libsdl.org/',
	'sqlite3'          : 'https://www.sqlite.org/',
	'zlib'             : 'https://zlib.net/',
}

# TODO Get version of these
also_embeded = [
	'base64',
	'roboto',
]

# TODO Save that in C++ structures
#if (static_dependencies.length() + also_embeded.length() > 0)
	about_embeded_dependencies = []
	last_embeded_dependencies = ''
	foreach dep, version: static_dependencies
		if last_embeded_dependencies != ''
			about_embeded_dependencies += last_embeded_dependencies
		endif
		last_embeded_dependencies = dep_fancy_name[dep]+' '+version+' ('+dep_website[dep]+')'
	endforeach
	foreach dep: also_embeded
		if last_embeded_dependencies != ''
			about_embeded_dependencies += last_embeded_dependencies
		endif
		last_embeded_dependencies = dep_fancy_name[dep]+' ('+dep_website[dep]+')'
	endforeach
	about_embeded_dependencies = ', '.join(about_embeded_dependencies) + ' and ' + last_embeded_dependencies
	message('This copy of Arcollect will embed copies of '+about_embeded_dependencies)
#endif	

dependency_report_data = configuration_data()
dependency_report_data.set_quoted('ABOUT_EMBEDED_DEPENDENCIES_STR',about_embeded_dependencies.replace('\n','\\n'))
dependency_report_hpp = configure_file(output: 'dependency-report.hpp', configuration: dependency_report_data)

# Add subdir (include order is important !)
subdir('i18n')
if enable_native_progs
	subdir('sqls')
	subdir('common')
	subdir('sqls/tests')
	subdir('desktop-app')
	subdir('webext-adder')
endif
if enable_webextension
	subdir('webextension')
endif
subdir('packaging')
