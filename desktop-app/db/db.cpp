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
#include "db.hpp"
#include <ctime>
std::unique_ptr<SQLite3::sqlite3> Arcollect::database;
sqlite_int64 Arcollect::data_version = -1;
static sqlite_int64 private_data_version = 0;
const std::string Arcollect::db::artid_randomizer = "((art_artid+"+std::to_string(std::time(NULL))+")*2654435761) % 4294967296";

sqlite_int64 Arcollect::update_data_version(void)
{
	std::unique_ptr<SQLite3::stmt> stmt;
	database->prepare("PRAGMA data_version;",stmt); // TODO Error checking
	stmt->step(); // TODO Error checking
	return data_version = stmt->column_int64(0) + private_data_version;
}
void Arcollect::local_data_version_changed(void)
{
	private_data_version++;
}
