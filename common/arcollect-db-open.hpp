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

namespace Arcollect {
	namespace db {
		/** Open the database
		 * \param flags SQLite open flags (R/W by default)
		 * \return The SQLite database handle
		 *
		 * This function open the database, run startup pragma and bootstrap/upgrade
		 * the database.
		 */
		std::unique_ptr<SQLite3::sqlite3> open(int flags = SQLite3::OPEN_READWRITE|SQLite3::OPEN_CREATE);
		/** Open and reset the database for test units
		 * \return The SQLite database handle
		 *
		 * This function erase the data home, then call open(). It is intended for
		 * test-units that needs to erase the database before.
		 *
		 * It requires `$ARCOLLECT_DATA_HOME` to be set, else it prints a Bail Out!
		 * message and `std::exit(1)` to prevent accidental erase of the user
		 * collection.
		 * \todo Protect even more the user database by requiring also to set the
		 * `$ARCOLLECT_TEST_CAN_SAFELY_NUKE_COLLECTION` environment variable.
		 */
		std::unique_ptr<SQLite3::sqlite3> test_open(void);
	}
}

