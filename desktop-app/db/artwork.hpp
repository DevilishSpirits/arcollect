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
#include "../sdl2-hpp/SDL.hpp"
#include "../config.hpp"
#include "../gui/font.hpp"
#include "download.hpp"
#include <cstddef>
#include <filesystem>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#if HAS_SDL_OPENURL
	#define ARTWORK_HAS_OPEN_URL
#endif

namespace Arcollect {
	namespace db {
		typedef sqlite_int64 artwork_id;
		class artwork;
		class account;
		/** Artwork data class holder
		 *
		 * This class hold data about an artwork. It's used trough a std::shared_ptr
		 * to maintain one instance per artwork.
		 *
		 * It is instanciated on-demand trough Arcollect::db::artwork::query().
		 */
		class artwork {
			private:
				artwork(Arcollect::db::artwork_id art_id);
				// Cached DB infos
				sqlite_int64 data_version;
				void db_sync(void);
				Arcollect::config::Rating art_rating;
				sqlite3_int64 art_partof;
				sqlite3_int64 art_pageno;
				sqlite3_int64 art_savedate;
				std::unordered_map<std::string,std::vector<std::shared_ptr<account>>> linked_accounts;
				/** Query a column
				 * \param column name
				 * \return The string or an empty string if NULL
				 * \warning **`column` is sensitive to SQL injections!** This function
				 *          is only meant to be used with hardcoded strings.
				 */
				std::string get_db_string(const std::string& column);
				std::shared_ptr<download> data;
				std::shared_ptr<download> thumbnail;
			public:
				/** File to query in the artwork
				 */
				enum File {
					FILE_ARTWORK,
					FILE_THUMBNAIL,
				};
				// Delete copy constructor
				artwork(const artwork&) = delete;
				artwork& operator=(artwork&) = delete;
				/** The database artwork id
				 */
				const Arcollect::db::artwork_id art_id;
				
				std::shared_ptr<download> get_artwork(void) {
					return data;
				}
				std::shared_ptr<download> get_thumbnail(void) {
					return thumbnail;
				}
				std::shared_ptr<download> get(File file) {
					static std::shared_ptr<download> none;
					switch (file) {
						case FILE_ARTWORK:
							return data;
						case FILE_THUMBNAIL:
							return thumbnail;
						default:
							return none;
					}
				}
				
				std::string title(void) {
					return get_db_string("art_title");
				}
				std::string desc(void) {
					return get_db_string("art_desc");
				}
				std::string source(void) {
					return get_db_string("art_source");
				}
				inline const std::string &mimetype(void) {
					db_sync();
					return data->dwn_mimetype;
				}
				inline const sqlite3_int64 &partof(void) const {
					return art_partof;
				}
				inline const sqlite3_int64 &pageno(void) const {
					return art_pageno;
				}
				const sqlite3_int64 savedate(void) const {
					return art_savedate;
				}
				
				const std::vector<std::shared_ptr<account>> &get_linked_accounts(const std::string &link);
				const std::vector<std::shared_ptr<account>> &get_linked_accounts(void);
				
				#ifdef ARTWORK_HAS_OPEN_URL
				/** Open the artwork URL
				 */
				void open_url(void);
				#endif
				
				/** Query an artwork
				 * \param art_id The artwork identifier
				 * \return The artwork wrapped in a std::shared_ptr
				 * This function create or return a cached version of the #Arcollect::db::artwork.
				 */
				static std::shared_ptr<artwork> &query(Arcollect::db::artwork_id art_id);
		};
	}
}
