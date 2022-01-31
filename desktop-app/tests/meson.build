desktop_app_test_env = {
	'XDG_CONFIG_HOME': meson.current_build_dir(),
	'XDG_DATA_HOME': meson.current_build_dir(),
}

tap_tests = [
	'test-config',
	'test-mime-extract-charset',
	'test-search',
]

foreach test: tap_tests
	
	test(test, executable(test, test+'.cpp', dependencies: desktop_app_dep), protocol: 'tap', env: desktop_app_test_env+{
		'ARCOLLECT_DATA_HOME': meson.current_build_dir()/(test+'.data_home'),
	}, is_parallel: false)
endforeach
