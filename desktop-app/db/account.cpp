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
#include "account.hpp"
#include "db.hpp"
#include "download.hpp"
static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::account>> accounts_pool;

Arcollect::db::account::account(Arcollect::db::account_id arcoid) :
	arcoid(arcoid)
{
}
std::shared_ptr<Arcollect::db::account> &Arcollect::db::account::query(Arcollect::db::account_id arcoid)
{
	std::shared_ptr<Arcollect::db::account> &pointer = accounts_pool.try_emplace(arcoid).first->second;
	if (!pointer)
		pointer = std::shared_ptr<Arcollect::db::account>(new Arcollect::db::account(arcoid));
	return pointer;
}

void Arcollect::db::account::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT acc_name, acc_title, acc_url, acc_icon FROM accounts WHERE acc_arcoid = ?;",stmt); // TODO Error checking
		stmt->bind(1,arcoid);
		if (stmt->step() == SQLITE_ROW) {
			acc_name   = stmt->column_string(0);
			acc_title  = stmt->column_type(1) == SQLITE_NULL ? acc_name : stmt->column_string(1);
			acc_url    = stmt->column_string(2);
			acc_icon   = stmt->column_int64(3);
			data_version = Arcollect::data_version;
		} else {
		}
	}
}
std::unique_ptr<SDL::Texture> &Arcollect::db::account::get_icon(void)
{
	db_sync();
	return Arcollect::db::download::query(acc_icon)->query_image();
}
