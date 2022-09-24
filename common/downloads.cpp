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
#include "arcollect-db-downloads.hpp"
#include "arcollect-paths.hpp"
#include "arcollect-sqls.hpp"
#include <iostream>
void Arcollect::db::downloads::DownloadInfo::set_dwn_path(const std::filesystem::path& dir, const std::string_view& filename)
{
	// Set the dir
	dwn_path_write = dir;
	dwn_path_write += dir.preferred_separator;
	// Sanitize filename
	for (const char c: filename)
		if (((c >= 'A') && (c <= 'Z'))||((c >= 'a') && (c <= 'z'))||((c >= '0') && (c <= '9'))||(c == '.')||(c == '_')||(c == '-'))
			dwn_path_write += c;
}

Arcollect::db::downloads::Transaction::Transaction(std::unique_ptr<SQLite3::sqlite3> &database) : db(database.get())
{
	// TODO Checks
	db->prepare(Arcollect::db::sql::cache_query_by_source,query_cache_stmt);
	db->prepare(Arcollect::db::sql::downloads_unsource,unsource_stmt);
}
void Arcollect::db::downloads::Transaction::commit(void) noexcept
{
	// Remove deleted
	for (const std::filesystem::path &filename: deleted_files)
		std::filesystem::remove(Arcollect::path::arco_data_home/filename);
	// Clear files list
	created_files.clear();
	deleted_files.clear();
}
Arcollect::db::downloads::Transaction::~Transaction(void) noexcept
{
	// Remove saved
	for (const std::filesystem::path &filename: created_files)
		std::filesystem::remove(Arcollect::path::arco_data_home/filename);
}
Arcollect::db::downloads::DownloadInfo::DownloadInfo(std::unique_ptr<SQLite3::stmt> &query_cache_stmt)
: 
	dwn_id_write(query_cache_stmt->column_int64(0)),
	dwn_path_write(query_cache_stmt->column_string(3)),
	dwn_lastedit(query_cache_stmt->column_int64(1)),
	dwn_etag    (query_cache_stmt->column_null(2) ? std::string() : query_cache_stmt->column_string(2))
{
}
Arcollect::db::downloads::DownloadInfo Arcollect::db::downloads::Transaction::query_cache(const std::string_view &db_key)
{
	query_cache_stmt->reset();
	query_cache_stmt->bind(1,db_key);
	switch (query_cache_stmt->step()) {
		case SQLITE_ROW:
			// Cache hit!
			return Arcollect::db::downloads::DownloadInfo(query_cache_stmt);
		case SQLITE_DONE:
			// Cache miss
			return Arcollect::db::downloads::DownloadInfo();
		default: {
			std::cerr << "Failed to query cache (dwn_source = " << db_key << "): " << db->errmsg() << ". Assume cache miss." << std::endl;
		} return Arcollect::db::downloads::DownloadInfo();
	}
}
void Arcollect::db::downloads::Transaction::unsource(sqlite3_int64 dwn_id)
{
	unsource_stmt->reset();
	unsource_stmt->bind(1,dwn_id);
	unsource_stmt->step();
}
std::string Arcollect::db::downloads::Transaction::move_refs(sqlite3_int64 from, sqlite3_int64 to)
{
	std::unique_ptr<SQLite3::stmt> stmt;
	const char *zSql = Arcollect::db::sql::downloads_move_refs.data();
	int substep = 0;
	while (*zSql) {
		substep++;
		if (db->prepare(zSql,-1,stmt,zSql) != SQLITE_OK)
			return "Updating download ref " + std::to_string(from) + " to " + std::to_string(to) + ", failed to prepare substep " + std::to_string(substep) + ": " + std::string(db->errmsg()) + ".";
		if (stmt->bind(1,from) != SQLITE_OK)
			return "Updating download ref " + std::to_string(from) + " to " + std::to_string(to) + ", failed to bind the source reference: " + std::string(db->errmsg()) + ".";
		if (stmt->bind(2,to) != SQLITE_OK)
			return "Updating download ref " + std::to_string(from) + " to " + std::to_string(to) + ", failed to bind the target reference: " + std::string(db->errmsg()) + ".";
		if (stmt->step() != SQLITE_DONE)
			return "Updating download ref " + std::to_string(from) + " to " + std::to_string(to) + ", failed to run substep " + std::to_string(substep) + ": " + std::string(db->errmsg()) + ".";
	}
	return "";
}
std::string Arcollect::db::downloads::Transaction::write_cache(const std::string_view &db_key, const std::string_view &mimetype, Arcollect::db::downloads::DownloadInfo& infos)
{
	if (infos)
		unsource(infos.dwn_id());
	const std::filesystem::path filename  = infos.dwn_path().stem();
	const std::filesystem::path extension = infos.dwn_path().extension();
	std::filesystem::path new_path(infos.dwn_path());
	for (unsigned int i = 0; i < std::numeric_limits<decltype(i)>::max(); ++i) {
		// Ensure that we are not writing an erased file
		if (deleted_files.find(new_path) != deleted_files.end()) {
			// dwn_path duplicate, try another one
			new_path = infos.dwn_path();
			new_path += std::filesystem::path("-"+std::to_string(i));
			continue; // This download is still used in the database
		}
		// Perform transaction
		std::unique_ptr<SQLite3::stmt> downloads_new_entry_stmt;
		db->prepare(Arcollect::db::sql::downloads_new_entry,downloads_new_entry_stmt);
		if (db_key.empty())
			downloads_new_entry_stmt->bind_null(1);
		else downloads_new_entry_stmt->bind(1,db_key);
		downloads_new_entry_stmt->bind(2,infos.dwn_lastedit);
		if (infos.dwn_etag.empty())
			downloads_new_entry_stmt->bind_null(3);
		else downloads_new_entry_stmt->bind(3,infos.dwn_etag);
		const std::string path_string = new_path.string();
		downloads_new_entry_stmt->bind(4,path_string);
		downloads_new_entry_stmt->bind(5,mimetype);
		switch (downloads_new_entry_stmt->step()) {
			case SQLITE_ROW: {
				std::optional<sqlite3_int64> old_ref = infos.dwn_id_write;
				infos.dwn_id_write = downloads_new_entry_stmt->column_int64(0);
				infos.dwn_path_write = new_path;
				created_files.emplace_back(std::move(new_path));
				// Get a SQLITE_DONE
				if (downloads_new_entry_stmt->step() != SQLITE_DONE)
					return std::string("Failed to add cache entry in database (in SQLITE_ROW handling): "+std::string(db->errmsg()));
				// Move references
				if (old_ref) {
					std::string move_refs_result = move_refs(*old_ref,infos.dwn_id());
					if (!move_refs_result.empty())
						return move_refs_result;
					delete_cache(*old_ref);
				}
			} return std::string();
			case SQLITE_CONSTRAINT: {
				if (db->extended_errcode() == SQLITE_CONSTRAINT_UNIQUE) {
					const std::string_view errmsg(db->errmsg());
					if (errmsg.ends_with("dwn_path")) {
						// dwn_path duplicate, try another one
						new_path = infos.dwn_path();
						new_path.replace_filename(filename);
						new_path += "-";
						new_path += std::to_string(i);
						new_path += extension;
						continue; // This download is still used in the database
					}
				}
			} // falltrough;
			default:
				return std::string("Failed to add cache entry in database: "+std::string(db->errmsg()));
		}
	}
	// Okay we REALLY have a problem
	return std::string("You have too much files starting with "+infos.dwn_path().string()+" in your collection. How did you do that?");
}
bool Arcollect::db::downloads::Transaction::delete_cache(sqlite3_int64 dwn_id)
{
	std::unique_ptr<SQLite3::stmt> delete_download_stmt;
	db->prepare(Arcollect::db::sql::delete_download,delete_download_stmt);
	delete_download_stmt->bind(1,dwn_id);
	switch (delete_download_stmt->step()) {
		case SQLITE_ROW: {
			// Success
			deleted_files.emplace(delete_download_stmt->column_string(0));
		} return true;
		case SQLITE_DONE: {
			std::cerr << "Tried to delete non existant download (dwn_id = " << dwn_id << ")" << std::endl;
		} return false;
		case SQLITE_CONSTRAINT: {
			if (db->extended_errcode() == SQLITE_CONSTRAINT_FOREIGNKEY)
				return false; // This download is still used in the database
		} // falltrough;
		default: {
			std::cerr << "Failed to delete download (dwn_id = " << dwn_id << "): " << db->errmsg() << ", entry has been kept" << std::endl;
		} return false;
	}
}
