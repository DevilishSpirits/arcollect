deskapp_srcs += files(
	'dbus.cpp',
	'freedesktop-application.cpp',
	'gnome-shell-search-provider.cpp',
)
deskapp_deps += [
	dep_dbus,
]
install_data('me.d_spirits.arcollect.desktop', install_dir: get_option('datadir')/'applications')
configure_file(input: 'gnome-shell-search-provider.ini', output: 'arcollect-search-provider.ini', configuration: config_h, install: true, install_dir: get_option('datadir')/'gnome-shell/search-providers')
configure_file(input: 'dbus.service', output: config_h.get('ARCOLLECT_DBUS_NAME')+'.service', configuration: config_h, install: true, install_dir: get_option('datadir')/'dbus-1/services')

desktop_file_validate_prog = find_program('desktop-file-validate', required: false, native: true)
if desktop_file_validate_prog.found()
	test('desktop-file-validate', desktop_file_validate_prog, args: ['--warn-kde',files('me.d_spirits.arcollect.desktop')])
endif

main_cpp = 'main-xdg.cpp'
