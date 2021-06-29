webext_src = [
	configure_file(output: 'manifest.json', input: 'manifest.json.in', configuration: config_h),
	'background.js',
	'content-scripts/common.js',
	'content-scripts/artstation.com.js',
	'content-scripts/deviantart.com.js',
	'content-scripts/e621.net.js',
	'content-scripts/furaffinity.net.js',
]

# Application manifest
webext_conf = configuration_data()
webext_conf.merge_from(config_h)
webext_conf.set('ARCOLLECT_WEBEXT_ADDER_PATH', get_option('prefix') / get_option('libexecdir') / 'webext_adder_exe')
webext_appmanifest_firefox = configure_file(input: 'appmanifest-firefox.json.in', output: 'appmanifest-firefox-install.json', configuration: webext_conf)

# FIXME This is dirty
if enable_native_progs
	webext_conf_debug = configuration_data()
	webext_conf_debug.merge_from(config_h)
	webext_conf_debug.set('ARCOLLECT_WEBEXT_ADDER_PATH',webext_adder_exe.full_path())
	webext_appmanifest_firefox_debug = configure_file(input: 'appmanifest-firefox.json.in', output: 'appmanifest-firefox.json', configuration: webext_conf_debug)
endif

# TODO Windows support
# TODO macOS support
install_data(webext_appmanifest_firefox, rename: 'arcollect_webext_adder.json', install_dir: get_option('libdir') / 'mozilla/native-messaging-hosts')

webextension_target = custom_target('arcollect.zip', output : 'arcollect.zip', input : webext_src, command : [files('build.sh'), '@CURRENT_SOURCE_DIR@', meson.current_build_dir(), prog_zip, '@OUTPUT@', '@INPUT@'], build_by_default: true)

# Test with web-ext lint
prog_web_ext = find_program('web-ext', required: false)
prog_unzip   = find_program('unzip', required: false)
if prog_web_ext.found() and prog_unzip.found()
	test('web-ext-lint', files('web-ext-lint.sh'), args: [webextension_target, meson.project_build_root(), prog_unzip.full_path(), prog_web_ext.full_path()], suite: ['webextension'])
else
	warning('"web-ext" or "unzip" missing. Will not check linting.')
endif
