/* Arcollect -- An artwork collection manager
 * Copyright (C) 2021 DevilishSpirits (aka D-Spirits or Luc B.)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "arcollect-db-open.hpp"
#include "arcollect-paths.hpp"
#include <arcollect-sqls.hpp>
#include <iostream>

std::unique_ptr<SQLite3::sqlite3> Arcollect::db::open(int flags)
{
	SQLite3::initialize();
	static const std::filesystem::path db_path = Arcollect::path::arco_data_home / "db.sqlite3";
	std::unique_ptr<SQLite3::sqlite3> data_db;
	// TODO Backup the db
	int sqlite_open_code = SQLite3::open(db_path.u8string().c_str(),data_db,flags);
	if (sqlite_open_code) {
		std::cerr << "Failed to open \"" << db_path << "\": " << sqlite3_errstr(sqlite_open_code) << std::endl;
		std::abort();
	}
	// TODO Error checking
	if (data_db->exec(Arcollect::db::sql::boot.c_str())) {
		std::cerr << "Failed to run SQL boot script: " << data_db->errmsg() << std::endl;
	}
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
	schema_version_stmt.reset(); // Avoid SQLITE_LOCKED upon upgrades
	
	// Handle schema_version
	switch (schema_version) {
		default: {
			std::cerr << "Unknown schema_version " << schema_version << ". Continue but might expect bugs..." << std::endl;
		} break;
		case 1: {
			// Upgrade the database using 'upgrade_v2.sql'
			if (data_db->exec(Arcollect::db::sql::upgrade_v2.c_str())) {
				std::cerr << "Failed to upgrade DB \"" << db_path << "\" (upgrade_v2.sql): " << data_db->errmsg() << " Rollback." << std::endl;
				data_db->exec("ROLLBACK;");
			}
		}
		case 2: {
			// Up-to-date database. Do nothing
		} break;
		case 0: {
			// Bootstrap the database
			std::cerr << "Blank database, bootstrap it and wish you will find and enjoy artworks !" << std::endl;
			if (data_db->exec(Arcollect::db::sql::init.c_str())) {
				std::cerr << "Failed to init DB \"" << db_path << "\": " << data_db->errmsg() << std::endl;
			}
		} break;
	}
	return data_db;
}
