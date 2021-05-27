#include "arcollect-db-open.hpp"
#include "arcollect-paths.hpp"
#include <arcollect-db-schema.hpp>
#include <iostream>

std::unique_ptr<SQLite3::sqlite3> Arcollect::db::open(int flags)
{
	static const std::string db_path = Arcollect::path::arco_data_home / "db.sqlite3";
	std::unique_ptr<SQLite3::sqlite3> data_db;
	// TODO Backup the db
	int sqlite_open_code = SQLite3::open(db_path.c_str(),data_db,flags);
	if (sqlite_open_code) {
		std::cerr << "Failed to open \"" << db_path << "\": " << sqlite3_errstr(sqlite_open_code) << std::endl;
		std::abort();
	}
	// TODO Error checking
	// Check schema version and update the DB if required
	sqlite_int64 schema_version = 0;
	std::unique_ptr<SQLite3::stmt> schema_version_stmt;
	if (data_db->prepare("SELECT value FROM arcollect_infos WHERE key='schema_version';",schema_version_stmt)) {
		std::cerr << "Failed to prepare schema_version query. Assume that the database is empty and bootstrap it. Error message is " << data_db->errmsg() << std::endl;
	} else switch (schema_version_stmt->step()) {
		case SQLITE_ROW: {
			schema_version = schema_version_stmt->column_int64(0);
		} break;
		case SQLITE_DONE: {
			std::cerr << "WHAT ?! The schema_version query succeeded with no row. Assume the database is empty and bootstrap it. Maybe corrruption ??" << std::endl;
		} break;
		default: {
			std::cerr << "The schema_version query failed. Assume that the database is empty and bootstrap it. Error message is " << data_db->errmsg() << std::endl;
		} break;
	}
	switch (schema_version) {
		default: {
			std::cerr << "Unknow schema_version " << schema_version << ". Continue but might expect bugs..." << std::endl;
		} break;
		case 1: {
			// Up-to-date database. Do nothing
		} break;
		case 0: {
			// Bootstrap the database
			std::cerr << "Blank database, bootstrap it and wish you will find and enjoy artworks !" << std::endl;
			if (data_db->exec(Arcollect::db::schema::init.c_str())) {
				std::cerr << "Failed to init DB \"" << db_path << "\": " << data_db->errmsg() << std::endl;
			}
		} break;
	}
	return data_db;
}
