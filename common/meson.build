common_deps = [
	dep_sqlite3,
	cpp.find_library('Kernel32', required: false), # For AllocConsole()
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
common_link_args = [
]

common_lib = static_library('common', common_srcs, include_directories: common_incdirs, dependencies: common_deps, build_by_default: false)

if (host_machine.system() == 'windows') and (cpp.get_id() == 'gcc')
	# Statically link MinGW libs
	common_link_args += ['-static-libgcc','-static-libstdc++']
endif

common_dep = declare_dependency(link_with: common_lib, link_args: common_link_args, include_directories: common_incdirs, dependencies: common_deps)
