deskapp_srcs += files(
	'dbus.cpp',
	'freedesktop-application.cpp',
	'gnome-shell-search-provider.cpp',
)
deskapp_deps += [
	dep_dbus,
]
install_data('arcollect.desktop', install_dir: get_option('datadir')/'applications')
configure_file(input: 'gnome-shell-search-provider.ini', output: 'arcollect-search-provider.ini', configuration: config_h, install: true, install_dir: get_option('datadir')/'gnome-shell/search-providers')
configure_file(input: 'dbus.service', output: config_h.get('ARCOLLECT_DBUS_NAME')+'.service', configuration: config_h, install: true, install_dir: get_option('datadir')/'dbus-1/services')

main_cpp = 'main-xdg.cpp'
