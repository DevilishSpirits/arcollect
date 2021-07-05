
webext_adder_deps = [
	dep_curl,
	dep_rapidjson,
	dep_sqlite3,
]
webext_adder_srcs = [
	'adder.cpp',
	'main.cpp',
]

if (host_machine.system() == 'windows')
	# Link to Winsock under Windows
	webext_adder_deps += cpp.find_library('ws2_32')
endif

webext_adder_exe = executable('arcollect-webext-adder', webext_adder_srcs, link_with: common_lib, dependencies: webext_adder_deps, include_directories: common_incdirs, install: true, install_dir: get_option('libexecdir'))
subdir('tests')
