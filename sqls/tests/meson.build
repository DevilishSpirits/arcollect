simple_sql_tests = [
	'test-sql-prepare'
]

foreach test: simple_sql_tests
	workdir = meson.current_build_dir() / test+'-data-home'
	test(test,executable(test,test+'.cpp', db_schema_sources_target[1], dependencies: common_dep), env: {
		'ARCOLLECT_DATA_HOME': workdir,
	}, protocol: 'tap')
endforeach
