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
#include <functional>
#include <ostream>
#include <string>
#include <string_view>

namespace Arcollect {
	namespace db {
		namespace search {
			enum Token {
				TOK_IDENTIFIER,
				TOK_BLANK,
				TOK_EOL,
				TOK_INVALID,
			};
			/** Perform tokenization of search terms
			 * \return NULL on success, else the char that caused an error
			 */
			const char* tokenize(const char* search, std::function<bool(Token token, std::string_view value, void* data)> new_token, void* data);
			/** Build a SQLite statement that match the search
			 * \param[out] query Stream where to write the query. Usully an empty std::ostringstream.
			 * \param[out] query_bindings Target vector containing values of query placeholders. It's the things you should bind.
			 * 
			 * \return NULL on success, else the char that caused an error
			 *
			 * \warning query_bindings std::string_view are pointing directly into search param. The search param must be valid as long as query_bindings exist.
			 */
			const char* build_stmt(const char* search, std::ostream &query, std::vector<std::string_view> &query_bindings);
			/** Build a SQLite statement that match the search
			 * \param[out] stmt The SQLite stmt to prepare.
			 * 
			 * \return true on failure
			 */
			bool build_stmt(const char* search, std::unique_ptr<SQLite3::stmt> &stmt);
		}
	}
}
