# Roboto embed
roboto_proj = subproject('roboto')
inith_proj = subproject('inih', default_options: [
	'with_INIReader=true',
	'distro_install=false',
])

deskapp_deps = [
	dep_sdl2,
	dep_sdl2_image,
	dep_sdl2_ttf,
	dep_sqlite3,
	inith_proj.get_variable('INIReader_dep'),
]
deskapp_srcs = [
	'main.cpp',
	'config.cpp',
	'db/account.cpp',
	'db/artwork.cpp',
	'db/artwork-loader.cpp',
	'db/db.cpp',
	'db/filter.cpp',
	'gui/artwork-viewport.cpp',
	'gui/first-run.cpp',
	'gui/font.cpp',
	'gui/menu.cpp',
	'gui/slideshow.cpp',
	'gui/view-slideshow.cpp',
	'gui/view-grid.cpp',
	'gui/window-borders.cpp',
	roboto_proj.get_variable('gen_roboto_target'),
]

executable('arcollect', deskapp_srcs, dependencies: deskapp_deps, include_directories: common_incdirs, link_with: common_lib, install: true)
