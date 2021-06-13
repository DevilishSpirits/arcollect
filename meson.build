project('arcollect', 'cpp',
	# x.y Version numbering
	# | |
	# | +-- Build: Incremented to rebuild webextension and public packages.
	# |
	# +---- Minor: Will be incremented one day
	version: '0.3',
	meson_version: '>=0.57.1',
	default_options: [
		'cpp_std=c++17',
		'cpp_rtti=false',
	])

# Options
enable_webextension = get_option('enable_webextension')
enable_native_progs = get_option('enable_native_progs')

# Programs
prog_zip = find_program('zip', native: true, required: enable_webextension)
# Dependencies
if enable_native_progs
	dep_sdl2 = dependency('sdl2')
	dep_sdl2_image = dependency('SDL2_image')
	dep_sdl2_ttf   = dependency('SDL2_ttf', version: '>=2.0.12')
	dep_sqlite3 = dependency('sqlite3', version: '>=3.35.0')
	dep_rapidjson = dependency('rapidjson')
endif

# Configuration
config_h = configuration_data()
config_h.set('ARCOLLECT_VERSION',meson.project_version())
config_h.set('ARCOLLECT_WEBSITE','https://github.com/DevilishSpirits/arcollect')
config_h.set('ARCOLLECT_MOZILLA_WEBEXT_ID','arcollect@d-spirits.me')
foreach key: config_h.keys()
	config_h.set_quoted(key+'_STR',config_h.get(key))
		summary(key,config_h.get(key), section: 'config.h')
endforeach

configure_file(output: 'config.h', configuration: config_h)
config_h_inc = include_directories('.')

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
