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
/** \file arcollect-db-downloads.hpp
 *  \brief Shared downloads management routines
 *
 * The home of #Arcollect::db::downloads.
 */
#pragma once
#include <sqlite3.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <stdio.h>
#include <unordered_set>
#include <vector>
namespace Arcollect {
	namespace db {
		/** Downloads management routines
		 *
		 * Downloads is not trivial and both the desktop-app and the webext-adder
		 * needs a set of shared subroutines to correctly manage them.
		 *
		 * \warning DO NOT TRY to overwrite things without using this API!
		 *          You WILL break things or introduce possibly dramatic bugs.
		 *          Carefully read each function documentation! Many take and edit
		 *          an Arcollect::db::downloads::Downloads, ensure that you don't
		 *          use old values as any fieds may be overwritten.
		 */
		namespace downloads {
			class Transaction;
			/** Downloads infos
			 *
			 * This struture hold informations about an entry in the cache
			 * \warning Many functions take and edit this structure, ensure that you 
		 * *          don't use old values as any fields may be overwritten.
			 */
			class DownloadInfo {
				private:
					friend Transaction;
					/** The download ID writeable
					 *
					 * For #Transaction use.
					 */
					std::optional<sqlite3_int64> dwn_id_write;
				public:
					/** Query the download ID
					 *
					 * You never have to set this value yourself, it is entirely managed
					 * by the download management library
					 *
					 * \warning DO NOT call this function if operator bool() doesn't
					 *          returns true.
					 */
					sqlite3_int64 dwn_id(void) const {
						return *dwn_id_write;
					}
					/** Last change UNIX timestamp
					 *
					 * Used for [`If-Modified-Since`](https://developer.mozilla.org/fr/docs/Web/HTTP/Headers/If-Modified-Since).
					 */
					sqlite3_int64 dwn_lastedit;
					/** Etag
					 *
					 * Used for caching
					 */
					std::string dwn_etag;
					/** File path
					 *
					 * This path is relative to Arcollect::path::arco_data_home.
					 *
					 * \warning NEVER CACHE THE PATH!!! write_cache() and overwrite_cache()
					 *          may change this field, you would overwrite a file that
					 *          shouldn't and break the user collection!
					 */
					std::filesystem::path dwn_path;
					/** Return the if the download info is valid
					 * \return If dwn_id is set
					 *
					 * It is intended for use after Transaction::query_cache() to know if
					 * there was a cache hit.
					 */
					operator bool(void) const {
						return dwn_id_write.has_value();
					}
					/** Default constructor */
					DownloadInfo(void) = default;
					/** Constructor for Transaction::query_cache() usage */
					DownloadInfo(std::unique_ptr<SQLite3::stmt> &query_cache_stmt);
			};
			/** A download transaction
			 *
			 * This class wrap a single transaction in the SQLite database and the
			 * filesystem.
			 *
			 * It maintain a list of files edited and automatically revert changes if
			 * destroyed unless you commit() them to make them permanent, this does
			 * not commit changes in SQLite that must be commited before.
			 */
			class Transaction {
				private:
					struct path_hash {
						std::size_t operator()(const std::filesystem::path& path) const {
							return std::hash<std::filesystem::path::string_type>()(path.native());
						}
					};
					/** Reference to the database
					 */
					SQLite3::sqlite3 *const db;
					std::unique_ptr<SQLite3::stmt> query_cache_stmt;
					std::unique_ptr<SQLite3::stmt> unsource_stmt;
					/** List of deleted files
					 *
					 * This is a list of (logically) deleted files, they are really erased
					 * from the disk within a commit() that also clear this set.
					 */
					std::unordered_set<std::filesystem::path,path_hash> deleted_files;
					/** List of created files
					 *
					 * This is a list of created files, they are deleted when the object
					 * is destroyed. This set is cleared on a commit().
					 */
					std::vector<std::filesystem::path> created_files;
					/** Move references
					 * \return An empty string on success or the error message
					 *
					 * Cache entries are immutable, when we update a resource, we move
					 * existing references **that should be moved** using this function.
					 *
					 * \note Unless you know that you want to keep the ref, delete_cache()
					 *       the old ref afterward to cleanup bits. Don't be afraid to
					 *       break things, at worst that's a no-op.
					 */
					std::string move_refs(sqlite3_int64 from, sqlite3_int64 to);
				public:
					/** Query cache for an URL
					 * \param db_key The URL to query from (might not be the real one)
					 * \return A download info, cast to bool(true) on cache hit.
					 * \warning Any DB change invalidate the result, don't forget to lock
					 *          the database before calling query_cache().
					 */
					DownloadInfo query_cache(const std::string_view &db_key);
					/** Write a new entry in the cache
					 * \param db_key  The cached URL (might not be the real one)
					 * \param[in/out] infos Infos to write in the database.
					 *                Some infos will be overwritten with the effective
					 *                settings.
					 *                You MUST have set all fields.
					 * \return An empty string on success, the error message else
					 * \warning `dwn_path` and other settings may be overwritten by this
					 *          function. Don't copy and reuse any field before calling
					 *          this function!
					 * \warning The old dwn_id will be delete_cache().
					 */
					std::string write_cache(const std::string_view &db_key, const std::string_view &mimetype, DownloadInfo& infos);
					/** Attempt to delete a download
					 * \return `true` if the download will be removed upon a commit()
					 *
					 * It may delete it or not if used by another key.
					 * \warning It rely on SQLite foreign keys enforcement.
					 */
					bool delete_cache(sqlite3_int64 dwn_id);
					/** Unsource a download
					 * \return An empty string on success or the error message
					 *
					 * Unset the dwn_source field so a new download using the same key can
					 * be used instead.
					 */
					void unsource(sqlite3_int64 dwn_id);
					
					Transaction(std::unique_ptr<SQLite3::sqlite3> &database);
					/** Move constructor
					 */
					Transaction(Transaction&&) = default;
					/** Move-assigment constructor
					 *
					 * It deletes the copy-assigment constructor.
					 */
					Transaction& operator=(Transaction&&) = default;
					/** Commit transaction
					 *
					 * Commit changes to the filesystem. Must be called after a commit in
					 * the SQLite database. It delete files in the #deleted_files set and
					 * clear both #created_files and #deleted_files set.
					 *
					 * The transaction can be reused, it is in the same state as it was
					 * just created.
					 */
					void commit(void) noexcept;
					/** Transaction cleanups destructor
					 *
					 * It revert any uncommited changes and delete all files in the
					 * #created_files set.
					 *
					 * This is a cleanup mechanism for an aborted query to implement
					 * atomicity on the file system.
					 */
					~Transaction(void) noexcept;
			};
		}
	}
}
