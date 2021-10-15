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
#pragma once
#include <sqlite3.hpp>
#include <string>

namespace Arcollect {
	extern std::unique_ptr<SQLite3::sqlite3> database;
	/** Database data_version
	 * 
	 * This is a cached value of SQLite's `PRAGMA data_version;`. It is used to
	 * detect database changes and invalidate data when required.
	 *
	 * This value is updated by update_data_version().
	 */
	extern sqlite_int64 data_version;
	/** Arcollect's data_version
	 * 
	 * SQLite's `PRAGMA data_version;` is not incremented with changes made by the
	 * local process.
	 *
	 * This value is incremented by local_data_version_changed().
	 */
	extern sqlite_int64 private_data_version;
	
	/** Signal a local change in the database
	 * 
	 * This function update #data_version for changes within the database.
	 *
	 * SQLite doesn't update `PRAGMA data_version;` upon local change, so we do it
	 * in a private variable added to this value.
	 */
	void local_data_version_changed(void);
	
	/** Update #data_version
	 * 
	 * `PRAGMA data_version;`, return and store the result in #data_version.
	 *
	 * This function is regulary called.
	 */
	sqlite_int64 update_data_version(void);
}
