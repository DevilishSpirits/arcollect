project('arcollect','cpp', default_options: [
	'cpp_std=c++17',
	'cpp_rtti=false',
])

# Options
enable_webextension = get_option('enable_webextension')

# Programs
prog_zip = find_program('zip', native: true, required: enable_webextension)
# Dependencies
dep_sdl2 = dependency('sdl2')
dep_sdl2_image = dependency('SDL2_image')
dep_sdl2_ttf   = dependency('SDL2_ttf')
dep_sqlite3 = dependency('sqlite3', version: '>=3.35.0')

# Add subdir (include order is important !)
subdir('db-schema')
subdir('common')
subdir('desktop-app')
subdir('webext-adder')
if enable_webextension
	subdir('webextension')
endif
