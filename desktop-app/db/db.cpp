#include "db.hpp"
std::unique_ptr<SQLite3::sqlite3> Arcollect::database;
sqlite_int64 Arcollect::data_version = -1;

sqlite_int64 Arcollect::update_data_version(void)
{
	std::unique_ptr<SQLite3::stmt> stmt;
	database->prepare("PRAGMA data_version;",stmt); // TODO Error checking
	stmt->step(); // TODO Error checking
	return data_version = stmt->column_int64(0);
}
