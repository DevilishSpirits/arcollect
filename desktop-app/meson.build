# Roboto embed
roboto_proj = subproject('roboto')

deskapp_deps = [
	dep_sdl2,
	dep_sdl2_image,
	dep_sdl2_ttf,
	dep_sqlite3,
]
deskapp_srcs = [
	'main.cpp',
	'db/artwork.cpp',
	'db/db.cpp',
	'gui/artwork-viewport.cpp',
	'gui/font.cpp',
	'gui/slideshow.cpp',
	'gui/view-slideshow.cpp',
	'gui/view-grid.cpp',
	'gui/window-borders.cpp',
	roboto_proj.get_variable('gen_roboto_target'),
]

executable('arcollect', deskapp_srcs, dependencies: deskapp_deps, include_directories: common_incdirs, link_with: common_lib, install: true)
