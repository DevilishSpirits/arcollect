# Striped down version from the WrapDB
project('sqlite', 'c', version: '3.35.5')

thread_dep = dependency('threads')
cc = meson.get_compiler('c')
dl_dep = cc.find_library('dl', required : false)
m_dep = cc.find_library('m', required: false)

sqlite_args = [
	'-DSQLITE_DEFAULT_MEMSTATUS=0',
	'-DSQLITE_LIKE_DOESNT_MATCH_BLOBS',
	'-DSQLITE_MAX_EXPR_DEPTH=0',
	'-DSQLITE_OMIT_DEPRECATED',
	'-DSQLITE_OMIT_PROGRESS_CALLBACK',
	'-DSQLITE_OMIT_SHARED_CACHE',
	'-DSQLITE_USE_ALLOCA',
	# TODO '-DSQLITE_OMIT_AUTOINIT',
]
sqlite3_incdir = include_directories('.')
dep_sqlite3 = declare_dependency(link_with:
	static_library('sqlite3', 'sqlite3.c', include_directories: sqlite3_incdir, c_args : sqlite_args, dependencies : [thread_dep, dl_dep, m_dep], install: false)
,include_directories : sqlite3_incdir)
