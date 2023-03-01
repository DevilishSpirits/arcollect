# Striped down version from the WrapDB
project('sqlite', 'c')

thread_dep = dependency('threads')
cc = meson.get_compiler('c')
dl_dep = cc.find_library('dl', required : false)
m_dep = cc.find_library('m', required: false)

sqlite_allocator = '-DSQLITE_SYSTEM_MALLOC'
if cc.has_function('HeapCreate', prefix: '#include <Windows.h>')
	sqlite_allocator = '-DSQLITE_WIN32_MALLOC'
endif

sqlite_args = [
	'-DSQLITE_DEFAULT_FILE_PERMISSIONS=0600',
	'-DSQLITE_DEFAULT_MEMSTATUS=0',
	'-DSQLITE_DQS=0',
	'-DSQLITE_LIKE_DOESNT_MATCH_BLOBS',
	'-DSQLITE_MAX_EXPR_DEPTH=0',
	'-DSQLITE_OMIT_AUTHORIZATION',
	'-DSQLITE_OMIT_AUTOINIT',
	'-DSQLITE_OMIT_AUTORESET',
	'-DSQLITE_OMIT_AUTOVACUUM',
	'-DSQLITE_OMIT_BLOB_LITERAL',
	'-DSQLITE_OMIT_CASE_SENSITIVE_LIKE_PRAGMA',
	'-DSQLITE_OMIT_CHECK',
	'-DSQLITE_OMIT_COMPILEOPTION_DIAGS',
	'-DSQLITE_OMIT_COMPLETE',
	'-DSQLITE_OMIT_DECLTYPE',
	'-DSQLITE_OMIT_DEPRECATED',
	'-DSQLITE_OMIT_DESERIALIZE',
	'-DSQLITE_OMIT_EXPLAIN',
	'-DSQLITE_OMIT_GENERATED_COLUMNS',
	'-DSQLITE_OMIT_GET_TABLE',
	'-DSQLITE_OMIT_HEX_INTEGER',
	'-DSQLITE_OMIT_INCRBLOB',
	'-DSQLITE_OMIT_INTEGRITY_CHECK',
	'-DSQLITE_OMIT_INTROSPECTION_PRAGMAS',
	'-DSQLITE_OMIT_JSON',
	'-DSQLITE_OMIT_LOAD_EXTENSION',
	'-DSQLITE_OMIT_LOCALTIME',
	'-DSQLITE_OMIT_MEMORYDB',
	'-DSQLITE_OMIT_PROGRESS_CALLBACK',
	'-DSQLITE_OMIT_SCHEMA_PRAGMAS',
	'-DSQLITE_OMIT_SHARED_CACHE',
	'-DSQLITE_OMIT_TCL_VARIABLE',
	'-DSQLITE_OMIT_TEMPDB',
	'-DSQLITE_OMIT_TRACE',
	'-DSQLITE_OMIT_UTF16',
	'-DSQLITE_USE_ALLOCA',
	'-DSQLITE_THREADSAFE=0',
	sqlite_allocator,
]

sqlite_have_funcs = [
	['-DHAVE_FDATASYNC=1'         ,'fdatasync'         ,'#include <unistd.h>'],
	['-DHAVE_ISNAN=1'             ,'isnan'             ,'#include <math.h>'],
	['-DHAVE_LOCALTIME_R=1'       ,'localtime_r'       ,'#include <time.h>'],
	['-DHAVE_LOCALTIME_S=1'       ,'localtime_s'       ,'#include <time.h>'],
	['-DHAVE_MALLOC_USABLE_SIZE=1','malloc_usable_size','#include <malloc.h>'],
	['-DHAVE_STRCHRNUL=1'         ,'strchrnul'         ,'#include <string.h>'],
	['-DHAVE_USLEEP=1'            ,'usleep'            ,'#include <unistd.h>'],
	['-DHAVE_UTIME=1'             ,'utime'             ,'#include <utime.h>'],
]
foreach func: sqlite_have_funcs
	if cc.has_function(func[1],prefix: func[2])
		sqlite_args += func[0]
	endif
endforeach

sqlite3_incdir = include_directories('.')

sqlite3_version = cc.get_define('SQLITE_VERSION', include_directories: sqlite3_incdir, prefix: '#include <sqlite3.h>').strip('"')

dep_sqlite3 = declare_dependency(link_with:
	static_library('sqlite3', 'sqlite3.c', include_directories: sqlite3_incdir, c_args : sqlite_args, dependencies : [thread_dep, dl_dep, m_dep], install: false, build_by_default: false)
,include_directories : sqlite3_incdir, version: sqlite3_version)
