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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "artwork-collection.hpp"
#include "db.hpp"
#include "sorting.hpp"
#include <arcollect-db-downloads.hpp>
#include <arcollect-paths.hpp>
#include <arcollect-sqls.hpp>
Arcollect::db::artwork_collection::artwork_collection(void)
{
	while (!need_entries(4096));
	cache.shrink_to_fit();
}
Arcollect::db::artwork_collection::iterator Arcollect::db::artwork_collection::find(artwork_id id)
{
	iterator iter = begin();
	const iterator iter_end = end();
	for (; (iter != iter_end) && (*iter != id); ++iter);
	return iter;
}

Arcollect::db::artwork_collection::iterator Arcollect::db::artwork_collection::find_artid_randomized(artwork_id id, bool nearest)
{
	const Arcollect::db::SortingImpl &sorter = Arcollect::db::sorting(Arcollect::db::SORT_RANDOM);
	cache_size_type left = 0;
	cache_size_type size = cache.size();
	const artwork& target = *artwork::query(id);
	while (size) {
		size -= 1;
		size /= 2;
		auto current = left + size;
		const artwork& current_artwork = *artwork::query(cache[current]);
		if (target.art_id == current_artwork.art_id)
			// Match
			return begin()+current;
		else if (sorter.compare_arts(current_artwork,target))
			// Match is in the right part, update left
			left += size + 1;
		//else size will be shrink on next loop iteration, nothing to do
	}
	if (nearest)
		return begin()+left;
	else return end();
}

int Arcollect::db::artwork_collection::db_delete(void)
{
	std::unordered_set<sqlite_int64> downloads_to_remove;
	int code;
	if ((code = database->exec("BEGIN IMMEDIATE;")) != SQLITE_OK) {
		switch (code) {
			case SQLITE_BUSY: {
				std::cerr << "Deleting artworks, \"BEGIN IMMEDIATE;\" failed with SQLITE_BUSY. Another is writing on the database. Abort." << std::endl;
			} return SQLITE_BUSY;
			default: {
				std::cerr << "Deleting artworks, \"BEGIN IMMEDIATE;\" failed: " << database->errmsg() << ". Ignoring..." << std::endl;
			} break;
		}
	}
	std::unique_ptr<SQLite3::stmt> stmt;
	
	// Run the query
	if (database->prepare(Arcollect::db::sql::delete_artwork,stmt) != SQLITE_OK) {
		std::cerr << "Deleting artworks, failed to prepare: " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;");
		return SQLITE_ERROR;
	}
	for (artwork_id art_id: *this) {
		stmt->reset();
		if (stmt->bind(1,art_id) != SQLITE_OK) {
			std::cerr << "Deleting artworks, failed to bind art_artid: " << database->errmsg() << ". Rollback." << std::endl;
			database->exec("ROLLBACK;");
			return SQLITE_ERROR;
		}
		switch (stmt->step()) {
			case SQLITE_ROW: {
				downloads_to_remove.insert(stmt->column_int64(0));
				downloads_to_remove.insert(stmt->column_int64(1));
			} break;
			case SQLITE_DONE: {
				std::cerr << "Tried to delete non existing artwork " << art_id << ". Continue normally." << std::endl;
			} break;
		}
		if (stmt->step() != SQLITE_DONE) {
			std::cerr << "Deleting artworks, the step() did not SQLITE_DONE: " << database->errmsg() << ". Rollback." << std::endl;
			database->exec("ROLLBACK;"); // TODO Error checkings
			return SQLITE_ERROR;
		}
	}
	// Erase downloads
	Arcollect::db::download::Transaction dwn_transaction(database);
	for (sqlite3_int64 dwn_id: downloads_to_remove)
		Arcollect::db::download::delete_cache(dwn_id,dwn_transaction);
	
	// Commit changes
	if (database->exec("COMMIT;") != SQLITE_OK) {
		std::cerr << "Deleting artworks, failed to commit changes: " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;"); // TODO Error checkings
		return SQLITE_ERROR;
	}
	
	dwn_transaction.commit();
	std::cerr << "Artworks has been deleted" << std::endl;
	// Update data_version
	Arcollect::local_data_version_changed();
	return 0;
}
int Arcollect::db::artwork_collection::db_set_rating(Arcollect::config::Rating rating)
{
	int code;
	if ((code = database->exec("BEGIN IMMEDIATE;")) != SQLITE_OK) {
		switch (code) {
			case SQLITE_BUSY: {
				std::cerr << "Setting artworks rating, \"BEGIN IMMEDIATE;\" failed with SQLITE_BUSY. Another is writing on the database. Abort." << std::endl;
			} return SQLITE_BUSY;
			default: {
				std::cerr << "Setting artworks rating, \"BEGIN IMMEDIATE;\" failed: " << database->errmsg() << ". Ignoring..." << std::endl;
			} break;
		}
	}
	std::unique_ptr<SQLite3::stmt> stmt;
	
	// Run all substeps from 'delete_artwork.sql'
	if (database->prepare("UPDATE artworks SET art_rating = ? WHERE art_artid = ?;",stmt) != SQLITE_OK) {
		std::cerr << "Setting artworks ratings, failed to prepare: " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;");
		return SQLITE_ERROR;
	}
	if (stmt->bind(1,static_cast<sqlite_int64>(rating)) != SQLITE_OK) {
		std::cerr << "Setting artworks ratings, failed to bind art_rating: " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;");
		return SQLITE_ERROR;
	}
	for (artwork_id art_id: *this) {
		if (stmt->bind(2,art_id) != SQLITE_OK) {
			std::cerr << "Setting artworks ratings, failed to bind art_artid: " << database->errmsg() << ". Rollback." << std::endl;
			database->exec("ROLLBACK;");
			return SQLITE_ERROR;
		}
		if (stmt->step() != SQLITE_DONE) {
			std::cerr << "Setting artworks ratings, failed to run substep: " << database->errmsg() << ". Rollback." << std::endl;
			database->exec("ROLLBACK;"); // TODO Error checkings
			return SQLITE_ERROR;
		}
		stmt->reset();
	}
	
	// Commit changes
	if (database->exec("COMMIT;") != SQLITE_OK) {
		std::cerr << "Setting artworks ratings, failed to commit changes: " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;"); // TODO Error checkings
		return SQLITE_ERROR;
	}
	std::cerr << "Artworks ratings sets" << std::endl;
	// Update data_version
	Arcollect::local_data_version_changed();
	// Reset taint
	for (auto& art_id: *this) {
		artwork::query(art_id)->get_artwork()->reset_taint(rating);
		artwork::query(art_id)->get_thumbnail()->reset_taint(rating);
	}
	return 0;
}
