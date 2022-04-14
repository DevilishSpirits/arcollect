deskapp_deps = [
	common_dep,
	dep_lcms2,
	dep_oiio,
	dep_sdl2,
	dep_harfbuzz,
	dep_freetype,
	dep_sqlite3,
	dep_inih,
	i18n_deps['common'],
	i18n_deps['desktop_app'],
]
deskapp_srcs = [
	'config.cpp',
	'i18n.cpp',
	'art-reader/image.cpp',
	'art-reader/text.cpp',
	'art-reader/text-rtf.cpp',
	'db/account.cpp',
	'db/artwork.cpp',
	'db/artwork-loader.cpp',
	'db/db.cpp',
	'db/artwork-collection.cpp',
	'db/download.cpp',
	'db/search.cpp',
	'db/sorting.cpp',
	'gui/about.cpp',
	'gui/artwork-viewport.cpp',
	'gui/edit-art.cpp',
	'gui/font.cpp',
	'gui/main.cpp',
	'gui/menu.cpp',
	'gui/modal.cpp',
	'gui/rating-selector.cpp',
	'gui/scrolling-text.cpp',
	'gui/search-osd.cpp',
	'gui/slideshow.cpp',
	'gui/view-slideshow.cpp',
	'gui/view-grid.cpp',
	'gui/window-borders.cpp',
	dependency_report_hpp,
]
deskapp_additional_srcs = [
]

main_cpp = 'main.cpp'
query_font = 'gui/font-query-embed.cpp'
if with_xdg
	subdir('xdg')
endif

deskapp_srcs += [query_font]

if query_font == 'gui/font-query-embed.cpp'
	roboto_proj = subproject('roboto')
	deskapp_srcs += [
		roboto_proj.get_variable('gen_roboto_target')[0],
	]
	deskapp_additional_srcs += [
		roboto_proj.get_variable('gen_roboto_target')[1],
	]
endif

desktop_app_staticlib = static_library('desktop-app-staticlib', deskapp_srcs, deskapp_additional_srcs, dependencies: deskapp_deps, build_by_default: false)

desktop_app_dep = declare_dependency(sources: deskapp_additional_srcs, dependencies: deskapp_deps, link_with: desktop_app_staticlib)

arcollect_exe = executable('arcollect', main_cpp, dependencies: desktop_app_dep, win_subsystem: 'windows', install: true)

subdir('tests')
