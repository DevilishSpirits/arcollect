# Roboto embed
roboto_proj = subproject('roboto')

deskapp_deps = [
	common_dep,
	dep_sdl2,
	dep_sdl2_image,
	dep_sdl2_ttf,
	dep_sqlite3,
	dep_inih,
]
if with_mingw_std_threads
	deskapp_deps += subproject('mingw-std-threads').get_variable('std_threaddep')
endif
deskapp_srcs = [
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
	'gui/main.cpp',
	'gui/menu.cpp',
	'gui/modal.cpp',
	'gui/rating-selector.cpp',
	'gui/search-osd.cpp',
	'gui/slideshow.cpp',
	'gui/view-slideshow.cpp',
	'gui/view-grid.cpp',
	'gui/window-borders.cpp',
	roboto_proj.get_variable('gen_roboto_target')[0],
]
deskapp_additional_srcs = [
	db_schema_sources_target[1], # for <arcollect-db-schema.hpp>
	roboto_proj.get_variable('gen_roboto_target')[1],
]

main_cpp = 'main.cpp'
if with_xdg
	subdir('xdg')
endif

# Add MinGW std threads if needed
if with_mingw_std_threads
	deskapp_deps += subproject('mingw-std-threads').get_variable('std_threaddep')
endif

desktop_app_staticlib = static_library('desktop-app-staticlib', deskapp_srcs, deskapp_additional_srcs, dependencies: deskapp_deps)

desktop_app_dep = declare_dependency(sources: deskapp_additional_srcs, dependencies: deskapp_deps, link_with: desktop_app_staticlib)

arcollect_exe = executable('arcollect', main_cpp, dependencies: desktop_app_dep, win_subsystem: 'windows', install: true)

subdir('tests')
