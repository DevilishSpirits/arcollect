
deskapp_deps = [
	dep_sdl2,
	dep_sdl2_image,
	dep_sqlite3,
]
deskapp_srcs = [
	'main.cpp',
	'db/artwork.cpp',
	'db/db.cpp',
	'gui/artwork-viewport.cpp',
	'gui/view-slideshow.cpp',
]

executable('arcollect', deskapp_srcs, dependencies: deskapp_deps, include_directories: common_incdirs, link_with: common_lib)
