# Roboto embed
roboto_proj = subproject('roboto')

deskapp_deps = [
	dep_sdl2,
	dep_sdl2_image,
	dep_sdl2_ttf,
	dep_sqlite3,
	dep_inih,
]
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

if with_xdg
	install_data('arcollect.desktop', install_dir: get_option('datadir')/'applications')
endif

if get_option('sdl_threading').auto()
	sdl_threading_tests = {
		'std::mutex': 'mutex',
		'std::thread': 'thread',
		'std::lock_guard': 'mutex',
		'std::condition_variable': 'condition_variable',
	}
	sdl_threading = false
	foreach type, header: sdl_threading_tests
		if not cpp.has_header_symbol(header,type)
			sdl_threading = true
		endif
	endforeach
else
	sdl_threading = get_option('sdl_threading').enabled()
endif

if sdl_threading
	error('sdl_threading is a work in progress.')
endif

desktop_app_staticlib = static_library('desktop-app-staticlib', deskapp_srcs, deskapp_additional_srcs, dependencies: deskapp_deps, include_directories: common_incdirs, link_with: common_lib)

desktop_app_dep = declare_dependency(sources: deskapp_additional_srcs, dependencies: deskapp_deps, include_directories: common_incdirs, link_with: desktop_app_staticlib)

arcollect_exe = executable('arcollect', 'main.cpp', dependencies: desktop_app_dep, install: true)

subdir('tests')
