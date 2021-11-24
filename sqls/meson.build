db_schema_src = [
	'adder_insert_account.sql',
	'adder_insert_artwork.sql',
	'adder_insert_tag.sql',
	'delete_artwork.sql',
	'preload_artworks.sql',
]
db_schema_src_no_test_prepare = [
	'boot.sql',
	'init.sql',
	'upgrade_v2.sql',
]

db_schema_sources_exe = executable('gen-schema-sources', 'gen-schema-sources.cpp', install: false)
db_schema_sources_target = custom_target('schema-sources', input: [db_schema_src,db_schema_src_no_test_prepare], command: [db_schema_sources_exe, '@OUTPUT0@', '@OUTPUT1@', '@INPUT@'], output: ['sqls.cpp', 'arcollect-sqls.hpp'], install: false, build_by_default: true)
