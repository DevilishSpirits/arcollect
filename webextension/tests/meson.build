
common_test_deps = [
	webext_adder_exe,
]

webext_test_env  = environment({
	'ARCOLLECT_DEBUG': 'all',
	'ARCOLLECT_WEBEXT_ADDER_PATH': webext_adder_exe.full_path(),
	'ARCOLLECT_WEBEXT_ZIP_PATH': webextension_target.full_path(),
	'ARCOLLECT_TEST_DATA_HOME': meson.current_build_dir(),
	'UBLOCK_ORIGIN_XPI_PATH': meson.current_source_dir()/'ublock_origin-1.36.0-an+fx.xpi',
})
webextension_browser_test_prog = find_program('webextension-browser-test.py')

geckodriver_prog = find_program('geckodriver', required: false)
if geckodriver_prog.found()
	geckodriver_test_deps = [
		common_test_deps,
	]
	test_suite = ['firefox-webextension','webextension']
	test('firefox-furaffinity',webextension_browser_test_prog, args: ['browser_geckodriver','platform_furaffinity',files('../../webext-adder/tests/furaffinity.net.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
	
	if get_option('tests_nsfw')
		test('firefox-e621',webextension_browser_test_prog, args: ['browser_geckodriver','platform_e621',files('../../webext-adder/tests/e621.net.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
		test('firefox-e621-nsfw',webextension_browser_test_prog, args: ['browser_geckodriver','platform_e621',files('../../webext-adder/tests/e621.net-nsfw.json')], env: webext_test_env, protocol: 'tap', timeout: 600, depends: geckodriver_test_deps, suite: test_suite, is_parallel: false)
	endif
endif
