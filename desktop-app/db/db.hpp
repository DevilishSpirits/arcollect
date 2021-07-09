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
#include <ctime>
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
	
	namespace db {
		/** Seed used for #Arcollect::db::artid_randomizer
		 *
		 * It's set to std::time(NULL) at program init.
		 */
		extern const std::time_t artid_randomizer_seed;
		/** SQL select column statement for stable artwork order randomization
		 *
		 * To avoid complete reorganization of the slideshow grid with random sort
		 * when rerunning an SQL statement.
		 *
		 * This algo is inspired from the Knuth's multiplicative method hash with
		 * the addition of a well-know "random" quantity `time(NULL)` to simulate
		 * randomness.
		 *
		 * This generate a value to "ORDER BY" against that is computed by
		 * '((art_artid+time(NULL))*2654435761) % 4294967296' and should works for
		 * most collections.
		 */
		extern const std::string artid_randomizer;
		
		/** Apply #Arcollect::db::artid_randomizer locally
		 */
		inline sqlite_int64 artid_randomize(sqlite_int64 art_artid) {
			return ((art_artid+artid_randomizer_seed)*2654435761) % 4294967296;
		}
	}
}
