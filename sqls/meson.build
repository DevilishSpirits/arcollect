db_schema_src = [
	'adder_cache_account.sql',
	'adder_cache_artwork.sql',
	'adder_cache_comic.sql',
	'adder_cache_tag.sql',
	'adder_insert_account.sql',
	'adder_insert_acc_icon.sql',
	'adder_insert_art_acc_link.sql',
	'adder_insert_art_tag_link.sql',
	'adder_insert_artwork.sql',
	'adder_insert_comic.sql',
	'adder_insert_com_acc_link.sql',
	'adder_insert_com_tag_link.sql',
	'adder_insert_tag.sql',
	'adder_update_account.sql',
	'adder_update_artwork.sql',
	'adder_update_comic.sql',
	'adder_update_tag.sql',
	'allocate_comicid.sql',
	'artworks_flags_by_dwn_id.sql',
	'cache_mv_art_dwnid.sql',
	'cache_mv_art_dwnthumbnail.sql',
	'cache_query_by_source.sql',
	'delete_artwork.sql',
	'delete_download.sql',
	'downloads_new_entry.sql',
	'preload_artworks.sql',
]
db_schema_src_no_test_prepare = [
	'boot.sql',
	'init.sql',
	'upgrade_v2.sql',
	'upgrade_v3.sql',
]

db_schema_sources_exe = executable('gen-schema-sources', 'gen-schema-sources.cpp', install: false, build_by_default: false)
db_schema_sources_target = custom_target('schema-sources', input: [db_schema_src,db_schema_src_no_test_prepare], command: [db_schema_sources_exe, '@OUTPUT0@', '@OUTPUT1@', '@INPUT@'], output: ['sqls.cpp', 'arcollect-sqls.hpp'], install: false, build_by_default: false)
