
webext_adder_deps = [
	dep_curl,
	dep_rapidjson,
	dep_sqlite3,
]
webext_adder_srcs = [
	'main.cpp'
]

webext_adder_exe = executable('arcollect-webext-adder', webext_adder_srcs, link_with: common_lib, dependencies: webext_adder_deps, include_directories: common_incdirs, install: true, install_dir: get_option('libexecdir'))
