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
deskapp_srcs = files(
	'config.cpp',
	'db/account.cpp',
	'db/artwork.cpp',
	'db/artwork-loader.cpp',
	'db/db.cpp',
	'db/filter.cpp',
	'db/search.cpp',
	'gui/artwork-viewport.cpp',
	'gui/first-run.cpp',
	'gui/font.cpp',
	'gui/menu.cpp',
	'gui/modal.cpp',
	'gui/rating-selector.cpp',
	'gui/slideshow.cpp',
	'gui/view-slideshow.cpp',
	'gui/view-grid.cpp',
	'gui/window-borders.cpp',
)
deskapp_additional_srcs = [
	db_schema_sources_target[1], # for <arcollect-db-schema.hpp>
	roboto_proj.get_variable('gen_roboto_target'),
]

desktop_app_staticlib = static_library('desktop-app-staticlib', deskapp_srcs, deskapp_additional_srcs, dependencies: deskapp_deps, include_directories: common_incdirs, link_with: common_lib)

desktop_app_dep = declare_dependency(sources: deskapp_additional_srcs, dependencies: deskapp_deps, include_directories: common_incdirs, link_with: desktop_app_staticlib)

executable('arcollect', 'main.cpp', dependencies: desktop_app_dep, install: true)

subdir('tests')
