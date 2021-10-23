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
#include <arcollect-db-open.hpp>
#include <arcollect-sqls.hpp>
#include <iostream>
#include <filesystem>
#include <utility>

using namespace Arcollect::db::sql;
#define SQL(name) {#name,name}
static const std::pair<std::string_view,std::string_view> test_sqls[] = {
		SQL(adder_insert_account),
		SQL(adder_insert_artwork),
		SQL(adder_insert_tag),
		SQL(delete_artwork),
		SQL(preload_artworks),
};

int main(void)
{
	std::cout << "1.." << (sizeof(test_sqls)/sizeof(test_sqls[0])) << std::endl;
	// Check environment
	auto* arcollect_home = std::getenv("ARCOLLECT_DATA_HOME");
	if (!arcollect_home || (*arcollect_home == '\0')) {
		std::cout << "Bail out! Refuse to run with $ARCOLLECT_DATA_HOME unset\n/!\\ THIS WOULD ERASE YOUR WHOLE COLLECTION!!!" << std::endl;
		return 1;
	}
	std::filesystem::remove(std::filesystem::path(arcollect_home)/"db.sqlite3");
	std::unique_ptr<SQLite3::sqlite3> database = Arcollect::db::open();
	int test_num = 1;
	for (const auto& test_unit: test_sqls) {
		std::unique_ptr<SQLite3::stmt> stmt;
		const char *zSql = test_unit.second.data();
		int substep = 0;
		int sqlite_code;
		do {
			substep++;
			sqlite_code = database->prepare(zSql,-1,stmt,zSql);
		} while (*zSql && !sqlite_code);
		// This SQL has been tested
		if (sqlite_code == SQLITE_OK)
			std::cout << "ok " << test_num++ << " - " << test_unit.first << " # Prepared " << substep << " substeps" << std::endl;
		else std::cout << "not ok " << test_num++ << " - " << test_unit.first << ": " << database->errmsg() << " # Failed " << " at substep " << substep << " near " << zSql << std::endl;
	}
}
