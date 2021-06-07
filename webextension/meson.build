webext_src = [
	'manifest.json',
	'background.js',
	'content-scripts/common-head.js',
	'content-scripts/common-foot.js',
	'content-scripts/furaffinity.net.js',
]

# Application manifest
webext_conf = configuration_data()
webext_conf.set('ARCOLLECT_WEBEXT_ADDER_PATH', get_option('prefix') / get_option('libexecdir') / 'arcollect-webext-adder')

webext_conf_debug = configuration_data()
webext_conf_debug.set('ARCOLLECT_WEBEXT_ADDER_PATH',webext_adder_exe.full_path())

webext_appmanifest_firefox = configure_file(input: 'appmanifest-firefox.json.in', output: 'appmanifest-firefox-install.json', configuration: webext_conf)
webext_appmanifest_firefox_debug = configure_file(input: 'appmanifest-firefox.json.in', output: 'appmanifest-firefox.json', configuration: webext_conf_debug)

# TODO Windows support
# TODO macOS support
install_data(webext_appmanifest_firefox, rename: 'arcollect_webext_adder.json', install_dir: get_option('libdir') / 'mozilla/native-messaging-hosts')

custom_target('arcollect.zip', output : 'arcollect.zip', input : webext_src, command : [files('build.sh'), '@CURRENT_SOURCE_DIR@', prog_zip, '@OUTPUT@', '@INPUT@'], build_by_default: true)
