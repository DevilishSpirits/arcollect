db_schema_src = [
	'boot.sql',
	'init.sql',
]
db_schema_sources_exe = executable('gen-schema-sources', 'gen-schema-sources.cpp', install: false)
db_schema_sources_target = custom_target('schema-sources', input: db_schema_src, command: [db_schema_sources_exe, '@OUTPUT0@', '@OUTPUT1@', '@INPUT@'], output: ['db-schema.cpp', 'arcollect-db-schema.hpp'], install: false, build_by_default: true)
