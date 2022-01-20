
common_test_deps = [
	webext_adder_exe,
	webextension_target,
]

webext_test_env  = environment({
	'ARCOLLECT_DEBUG': 'all',
	'ARCOLLECT_WEBEXT_ADDER_PATH': webext_adder_exe.full_path(),
	'ARCOLLECT_WEBEXT_ZIP_PATH': webextension_target.full_path(),
	'ARCOLLECT_TEST_DATA_HOME': meson.current_build_dir(),
	'LANG': 'C',
	'LC_ALL': 'C',
	'HOME': meson.current_build_dir(),
	'FIREFOX_APPMANIFEST_SOURCE': webext_appmanifest_firefox_debug,
	'FIREFOX_APPMANIFEST_DEST': '.mozilla/native-messaging-hosts/arcollect_webext_adder.json',
	'UBLOCK_ORIGIN_XPI_PATH': meson.current_source_dir()/'uBlock0_1.40.8.firefox.signed.xpi',
})
webextension_browser_test_prog = find_program('webextension-browser-test.py')

if (host_machine.system() == 'windows')
	# We won't mess the user registry to make the test works
	geckodriver_prog = disabler()
else
	geckodriver_prog = find_program('geckodriver', required: false)
endif

if geckodriver_prog.found()
	geckodriver_test_deps = [
		common_test_deps,
	]
	test_suite = ['firefox-webextension','webextension']
	test('firefox-furaffinity',webextension_browser_test_prog, args: ['browser_geckodriver','platform_furaffinity',files('../../webext-adder/tests/furaffinity.net.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
	test('firefox-artstation',webextension_browser_test_prog, args: ['browser_geckodriver','platform_artstation',files('../../webext-adder/tests/artstation.com.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
	test('firefox-deviantart',webextension_browser_test_prog, args: ['browser_geckodriver','platform_deviantart',files('../../webext-adder/tests/deviantart.com.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
	
	if get_option('tests_nsfw')
		test('firefox-e621',webextension_browser_test_prog, args: ['browser_geckodriver','platform_e621',files('../../webext-adder/tests/e621.net.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
		test('firefox-e621-nsfw',webextension_browser_test_prog, args: ['browser_geckodriver','platform_e621',files('../../webext-adder/tests/e621.net-nsfw.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
	endif
endif
