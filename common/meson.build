dep_sqlite3 = dependency('sqlite3')

common_deps = [
	dep_sqlite3,
]
common_srcs = [
	'paths.cpp',
	'db-open.cpp',
	db_schema_sources_target,
]
common_incdirs = [
	include_directories('.'),
	include_directories('sqlite3-hpp'),
]

common_lib = static_library('common', common_srcs, include_directories: common_incdirs, dependencies: common_deps)
