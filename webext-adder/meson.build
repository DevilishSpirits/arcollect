
webext_adder_deps = [
	common_dep,
	dep_curl,
	dep_sqlite3,
]
webext_adder_srcs = [
	'adder.cpp',
	'main.cpp',
	db_schema_sources_target[1], # for <arcollect-sqls.hpp>
]

if (host_machine.system() == 'windows')
	# Link to Winsock under Windows
	webext_adder_deps += cpp.find_library('ws2_32')
endif

webext_adder_exe = executable('arcollect-webext-adder', webext_adder_srcs, dependencies: webext_adder_deps, install: true, install_dir: get_option('libexecdir'))

# Application manifest for Firefox
webext_conf = configuration_data()
webext_conf.merge_from(config_h)
webext_conf.set('ARCOLLECT_WEBEXT_ADDER_PATH', get_option('prefix') / get_option('libexecdir') / 'arcollect-webext-adder')

webext_appmanifest_firefox = configure_file(input: 'appmanifest-firefox.json.in', output: 'appmanifest-firefox-install.json', configuration: webext_conf)

webext_conf_inbuild = configuration_data()
webext_conf_inbuild.merge_from(config_h)
webext_conf_inbuild.set('ARCOLLECT_WEBEXT_ADDER_PATH',webext_adder_exe.full_path())
webext_appmanifest_firefox_debug = configure_file(input: 'appmanifest-firefox.json.in', output: 'appmanifest-firefox.json', configuration: webext_conf_inbuild)
webext_appmanifest_firefox_debug = meson.current_build_dir()/'appmanifest-firefox.json'

webext_conf_windows = configuration_data()
webext_conf_windows.merge_from(config_h)
webext_conf_windows.set('ARCOLLECT_WEBEXT_ADDER_PATH','arcollect-webext-adder.exe')

webext_appmanifest_firefox_windows = configure_file(input: 'appmanifest-firefox.json.in', output: 'appmanifest-firefox-windows.json', configuration: webext_conf_windows)
webext_appmanifest_firefox_windows_full_path = meson.current_build_dir()/'appmanifest-firefox-windows.json'

if host_machine.system() == 'windows'
	webext_appmanifest_firefox_installdir = ''
elif host_machine.system() == 'darwin'
	webext_appmanifest_firefox_installdir = '/Library/Application Support/Mozilla/NativeMessagingHosts/'
else # Assume Linux-like
	webext_appmanifest_firefox_installdir = get_option('libdir') / 'mozilla/native-messaging-hosts'
endif

install_data(webext_appmanifest_firefox, rename: 'arcollect_webext_adder.json', install_dir: webext_appmanifest_firefox_installdir)

subdir('tests')
