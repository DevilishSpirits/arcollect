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
#include <deque>
#include <string>
#include <string_view>
#include <variant>

namespace Arcollect {
	namespace gui {
		namespace font {
			class Elements;
		}
	}
	namespace db {
		enum SearchType {
			SEARCH_ARTWORKS,
		};
		enum SortingType {
			/** Use a pseudo random sorting
			 *
			 * This algo is inspired from the Knuth's multiplicative method hash with
			 * the addition of the well-know "random" quantity `time(NULL)` to
			 * simulate randomness.
			 *
			 * It avoid a complete reorganization of the slideshow grid with random
			 * sort when rerunning an SQL statementsss.
			 *
			 * This generate a value to "ORDER BY" against that is computed by
			 * '((art_artid+time(NULL))*2654435761) % 4294967296' and should works
			 * for most collections.
			 *
			 * This is the original Arcollect sorting system.
			 */
			SORT_RANDOM,
		};
		struct SortingImpl;
		const SortingImpl& sorting(SortingType mode);
		class artwork_collection;
	}
	/** Search engine
	 *
	 * This namespace defined the search generation engine, it is inspired from
	 * the e621 posts searching system.
	 */
	namespace search {
		using Arcollect::db::SearchType;
		using Arcollect::db::SortingType;
		using Arcollect::db::SortingImpl;
		struct ParsedSearch {
			private:
				/** The generated SQL query
				 */
				std::string sql_query;
				/** Bindings of the generated SQL query
				 *
				 * Parameters bindings
				 */
				std::deque<std::variant<std::string_view,sqlite_int64>> sql_bindings;
				/** Text elements
				 */
				std::unique_ptr<Arcollect::gui::font::Elements> cached_elements;
			public:
				using sql_bindings_type = decltype(sql_bindings);
				/** The original search
				 *
				 * This string is the storage and most std::string_view in other
				 * elements.
				 */
				const std::string search;
				/** The kind of data returned by the search
				 */
				const SearchType search_type;
				/** The sorting type of entries
				 */
				const SortingType sorting_type;
				/** Sorting implementation
				 */
				const SortingImpl &sorting(void) const {
					return Arcollect::db::sorting(sorting_type);
				}
				/** Prepare a SQLite stmt
				 * \param[out] stmt The output stmt
				 * \warning The stmt link on std::string_view stored in #search and will
				 *          be invalid when the search is destroyed.
				 */
				void build_stmt(std::unique_ptr<SQLite3::stmt> &stmt) const;
				/** Prepare a SQLite stmt
				 * \param[out] stmt The output stmt
				 * \warning The stmt link on std::string_view stored in #search and will
				 *          be invalid when the search is destroyed.
				 */
				std::shared_ptr<Arcollect::db::artwork_collection> make_shared_collection(void) const;
				/** Return the colorized search
				 * \return A reference to the colorized Arcollect::gui::font::Elements.
				 */
				const Arcollect::gui::font::Elements &elements(void) const {
					return *cached_elements;
				}
				/** String move constructor
				 * \param[in] search_terms The search string to steal.
				 */
				ParsedSearch(std::string &&search_terms, SearchType search_type, SortingType sorting_type);
				/** String copy constructor
				 *
				 */
				ParsedSearch(const std::string_view search_terms, SearchType search_type, SortingType sorting_type) : ParsedSearch(std::string(search_terms),search_type,sorting_type) {}
				/** Default constructor
				 *
				 * The default constructor build a search of all artworks in random
				 */
				ParsedSearch(void) : ParsedSearch(std::string_view(""),db::SEARCH_ARTWORKS,db::SORT_RANDOM) {}
				/** Delete copy constructor
				 *
				 * There is references that cannot be moved
				 */
				ParsedSearch(const ParsedSearch&) = delete;
				/** Move constructor
				 *
				 * Move operations are fines.
				 */
				ParsedSearch(ParsedSearch&&) = default;
		};
	}
}
