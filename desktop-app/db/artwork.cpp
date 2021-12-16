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
#include "artwork.hpp"
#include "artwork-loader.hpp"
#include "account.hpp"
#include "db.hpp"

static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::artwork>> artworks_pool;
extern SDL::Renderer *renderer;

Arcollect::db::artwork::artwork(Arcollect::db::artwork_id art_id) :
	data_version(-2),
	art_id(art_id)
{
	db_sync();
}
std::shared_ptr<Arcollect::db::artwork> &Arcollect::db::artwork::query(Arcollect::db::artwork_id art_id)
{
	std::shared_ptr<Arcollect::db::artwork> &pointer = artworks_pool.try_emplace(art_id).first->second;
	if (!pointer)
		pointer = std::shared_ptr<Arcollect::db::artwork>(new Arcollect::db::artwork(art_id));
	return pointer;
}

static std::string column_string_default(std::unique_ptr<SQLite3::stmt> &stmt, int col)
{
	if (stmt->column_type(col) == SQLITE_NULL)
		return "";
	else return stmt->column_string(col);
}

void Arcollect::db::artwork::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT art_rating, art_dwnid, art_thumbnail, art_partof, art_pageno FROM artworks WHERE art_artid = ?;",stmt); // TODO Error checking
		stmt->bind(1,art_id);
		if (stmt->step() == SQLITE_ROW) {
			art_rating = static_cast<Arcollect::config::Rating>(stmt->column_int64(0));
			data      = download::query(stmt->column_int64(1));
			thumbnail = download::query(stmt->column_int64(2));
			art_partof = stmt->column_int64(3);
			art_partof = stmt->column_int64(4);
			
			data_version = Arcollect::data_version;
		}
		
		linked_accounts.clear();
	}
}
std::string Arcollect::db::artwork::get_db_string(const std::string& column)
{
	std::unique_ptr<SQLite3::stmt> stmt;
	const std::string query = "SELECT "+column+" FROM artworks WHERE art_artid = ?;";
	if (database->prepare(query.c_str(),stmt)) {
		std::cerr << "Failed to prepare '" << query << "': " << database->errmsg() << std::endl;
		return "";
	}
	stmt->bind(1,art_id);
	if (stmt->step() == SQLITE_ROW) {
		return column_string_default(stmt,0);
	} else {
		std::cerr << "Failed to get " << column << " column from artwork " << art_id << ": " << database->errmsg() << std::endl;
		return "";
	}
}

const std::vector<std::shared_ptr<Arcollect::db::account>> &Arcollect::db::artwork::get_linked_accounts(const std::string &link)
{
	db_sync();
	auto iterbool = linked_accounts.emplace(link,std::vector<std::shared_ptr<account>>());
	std::vector<std::shared_ptr<account>> &result = iterbool.first->second;
	if (iterbool.second) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT acc_arcoid FROM art_acc_links WHERE art_artid = ? AND artacc_link = ?;",stmt); // TODO Error checking
		;
		stmt->bind(1,art_id);
		stmt->bind(2,link.c_str());
		while (stmt->step() == SQLITE_ROW) {
			result.emplace_back(Arcollect::db::account::query(stmt->column_int64(0)));
			
			data_version = Arcollect::data_version;
		}
	}
	return result;
}

#ifdef ARTWORK_HAS_OPEN_URL
void Arcollect::db::artwork::open_url(void)
{
	SDL_OpenURL(source().c_str());
}
#endif
