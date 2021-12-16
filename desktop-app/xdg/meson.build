fs = import('fs')

	# https://specifications.freedesktop.org/menu-spec/latest/apa.html
xdg_menu_categories = [
	# Main categories
	'Graphics',
	# Additional categories
	'Art',
	'2DGraphics',
	'Amusement',
	'RasterGraphics',
	'Viewer',
]
deskapp_srcs += files(
	'dbus.cpp',
	'freedesktop-application.cpp',
	'gnome-shell-search-provider.cpp',
)
deskapp_deps += [
	dep_dbus,
]

# Generate both files in one executable to reduce release build time
xdg_localized_files = custom_target('xdg-localized-files', output: [
		'me.d_spirits.arcollect.desktop',
		config_h.get('ARCOLLECT_DBUS_NAME')+'.metainfo.xml',
	], command: [
		executable('generate-xdg-files',files(
			'generate-xdg-files.cpp',
			'generate-desktop-file.cpp',
			'generate-metainfo-xml.cpp',
			),include_directories: config_h_inc, install: false, build_by_default: false, dependencies: i18n_deps['common']
		),
		'@OUTPUT@',
	],env: {
		'DESKTOP_Categories': ';'.join(xdg_menu_categories),
		'APPSTREAM_categories': '<category>'+'</category>\n\t\t<category>'.join(xdg_menu_categories)+'</category>',
	},install: true, install_dir: [
		get_option('datadir')/'applications',
		get_option('datadir')/'metainfo',
	])
desktop_file   = xdg_localized_files[0]
appstream_file = xdg_localized_files[1]

configure_file(input: 'gnome-shell-search-provider.ini', output: 'arcollect-search-provider.ini', configuration: config_h, install: true, install_dir: get_option('datadir')/'gnome-shell/search-providers')
configure_file(input: 'dbus.service', output: config_h.get('ARCOLLECT_DBUS_NAME')+'.service', configuration: config_h, install: true, install_dir: get_option('datadir')/'dbus-1/services')

desktop_file_validate_prog = find_program('desktop-file-validate', required: false, native: true)
if desktop_file_validate_prog.found()
	test('desktop-file-validate', desktop_file_validate_prog, args: ['--warn-kde',desktop_file.full_path()])
endif
appstream_util_prog = find_program('appstream-util', required: false, native: true)
if appstream_util_prog.found()
	test('appstream-util-metainfo-file-validate', appstream_util_prog, args: ['validate-relax','--nonet',appstream_file.full_path()])
endif
appstreamcli_prog = find_program('appstreamcli', required: false, native: true)
if appstreamcli_prog.found()
	test('appstreamcli-metainfo-file-validate', appstreamcli_prog, args: ['validate','--nonet',appstream_file.full_path()])
endif

main_cpp = 'main-xdg.cpp'
