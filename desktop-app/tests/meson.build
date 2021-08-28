desktop_app_test_env = environment({
	'XDG_CONFIG_HOME': meson.current_build_dir(),
	'XDG_DATA_HOME': meson.current_build_dir(),
})

tap_tests = [
	'test-config',
	'test-search-tokenizer',
	'test-mime-extract-charset',
]

foreach test: tap_tests
	test(test, executable(test, test+'.cpp', dependencies: desktop_app_dep), protocol: 'tap', env: desktop_app_test_env, is_parallel: false)
endforeach
