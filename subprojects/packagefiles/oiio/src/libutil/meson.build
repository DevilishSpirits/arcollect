libOpenImageIO_Util_srcs = [
	'benchmark.cpp',
	'errorhandler.cpp',
	'filesystem.cpp',
	'farmhash.cpp',
	'filter.cpp',
	'hashes.cpp',
	'paramlist.cpp',
	'plugin.cpp',
	'SHA1.cpp',
	'strutil.cpp',
	'sysutil.cpp',
	'thread.cpp',
	'timer.cpp',
	'typedesc.cpp',
	'ustring.cpp',
	'xxhash.cpp',
]
OpenImageIO_Util_deps = [
	dep_boost,
	dep_imath,
	dep_fmt,
]
OpenImageIO_Util = static_library(libOpenImageIO_Util_srcs, dependencies: OpenImageIO_Util_deps, include_directories: include_directories('../include'), build_by_default: false)
