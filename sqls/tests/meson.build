simple_sql_tests = [
	'test-sql-prepare'
]

test_prepare = []
foreach sql: db_schema_src
	test_prepare += sql.substring(0,-4)
endforeach
test('test-sql-prepare',
	executable('test-sql-prepare',
		configure_file(input: 'test-sql-prepare.cpp', output: 'test-sql-prepare.cpp', configuration: {
			'DB_SCHEMA_SRC_TEST_PREPARE': 'SQL('+'),\n\tSQL('.join(test_prepare)+')\n',
		}), dependencies: common_dep
	), env: {
	'ARCOLLECT_DATA_HOME': meson.current_build_dir() / 'test-sql-prepare-data-home',
}, protocol: 'tap')

foreach test: simple_sql_tests
	workdir = meson.current_build_dir() / test+'-data-home'
endforeach
