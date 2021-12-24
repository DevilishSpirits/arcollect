simple_common_tests = [
	'test-download-scenario0',
]

foreach test: simple_common_tests
	workdir = meson.current_build_dir() / test+'-data-home'
	test(test,executable(test,test+'.cpp', dependencies: common_dep), env: {
		'ARCOLLECT_DATA_HOME': workdir,
	}, protocol: 'tap')
endforeach
