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

desktop_conf = configuration_data()
desktop_conf.merge_from(config_h)
desktop_conf.set('DESKTOP_Categories',';'.join(xdg_menu_categories))
desktop_file = configure_file(input: 'me.d_spirits.arcollect.desktop', output: 'me.d_spirits.arcollect.desktop', configuration: desktop_conf, install: true, install_dir: get_option('datadir')/'applications')

appstream_conf = configuration_data()
appstream_conf.merge_from(config_h)
appstream_conf.set('APPSTREAM_categories','<category>'+'</category>\n\t\t<category>'.join(xdg_menu_categories)+'</category>')
appstream_conf.set('APPSTREAM_desktop_NAME',fs.name(desktop_file))
appstream_file = configure_file(input: 'arcollect.metainfo.xml', output: config_h.get('ARCOLLECT_DBUS_NAME')+'.metainfo.xml', configuration: appstream_conf, install: true, install_dir: get_option('datadir')/'metainfo')
configure_file(input: 'gnome-shell-search-provider.ini', output: 'arcollect-search-provider.ini', configuration: config_h, install: true, install_dir: get_option('datadir')/'gnome-shell/search-providers')
configure_file(input: 'dbus.service', output: config_h.get('ARCOLLECT_DBUS_NAME')+'.service', configuration: config_h, install: true, install_dir: get_option('datadir')/'dbus-1/services')

desktop_file_validate_prog = find_program('desktop-file-validate', required: false, native: true)
if desktop_file_validate_prog.found()
	test('desktop-file-validate', desktop_file_validate_prog, args: ['--warn-kde',desktop_file])
endif
appstream_util_prog = find_program('appstream-util', required: false, native: true)
if appstream_util_prog.found()
	test('appstream-util-metainfo-file-validate', appstream_util_prog, args: ['validate-relax','--nonet',appstream_file])
endif
appstreamcli_prog = find_program('appstreamcli', required: false, native: true)
if appstreamcli_prog.found()
	test('appstreamcli-metainfo-file-validate', appstreamcli_prog, args: ['validate','--nonet',appstream_file])
endif

main_cpp = 'main-xdg.cpp'
