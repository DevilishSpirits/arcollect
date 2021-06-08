project('arcollect','cpp', default_options: [
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
dep_sdl2_ttf   = dependency('SDL2_ttf')
dep_sqlite3 = dependency('sqlite3', version: '>=3.35.0')
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
