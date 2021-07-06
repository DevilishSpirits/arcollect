common_deps = [
	dep_sqlite3,
]
common_srcs = [
	'db-open.cpp',
	'debug.cpp',
	'paths.cpp',
	db_schema_sources_target,
]
common_incdirs = [
	include_directories('.'),
	include_directories('sqlite3-hpp'),
	config_h_inc,
]

common_lib = static_library('common', common_srcs, include_directories: common_incdirs, dependencies: common_deps)

common_dep = declare_dependency(link_with: common_lib, include_directories: common_incdirs, dependencies: common_deps)
