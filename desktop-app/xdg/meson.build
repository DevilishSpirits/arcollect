deskapp_srcs += files(
	'dbus.cpp',
	'gnome-shell-search-provider.cpp',
)
deskapp_deps += [
	dep_dbus,
]
install_data('arcollect.desktop', install_dir: get_option('datadir')/'applications')
configure_file(input: 'gnome-shell-search-provider.ini', output: 'arcollect-search-provider.ini', configuration: config_h, install: true, install_dir: get_option('datadir')/'gnome-shell/search-providers')

main_cpp = 'main-xdg.cpp'
