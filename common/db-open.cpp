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
	// Bootstrap the DB
	/* TODO 
	std::unique_ptr<SQLite3::stmt> schema_version_stmt;
	data_db->prepare("")
	*/
	int sqlite_init_code = data_db->exec(Arcollect::db::schema::init.c_str());
	if (sqlite_init_code) {
		std::cerr << "Failed to init DB \"" << db_path << "\": " << data_db->errmsg() << std::endl;
	}
	return data_db;
}
