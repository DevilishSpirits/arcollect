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
#include "search.hpp"
#include "db.hpp"
#include "filter.hpp"
#include <cctype>
#include <sstream>

// TODO UTF-8 support
static Arcollect::db::search::Token classify_char(char chr)
{
	if (chr == '\0')
		return Arcollect::db::search::TOK_EOL;
	else if (std::isalnum(chr) || (chr == '_'))
		return Arcollect::db::search::TOK_IDENTIFIER;
	/*else if (chr == ':')
		return Arcollect::db::search::TOK_COLON;*/
	else if (chr == '-')
		return Arcollect::db::search::TOK_NEGATE;
	else if (std::isblank(chr))
		return Arcollect::db::search::TOK_BLANK;
	else return Arcollect::db::search::TOK_INVALID;
}
const char* Arcollect::db::search::tokenize(const char* search, std::function<bool(Token token, std::string_view value, void* data)> new_token, void* data)
{
	Token state = TOK_BLANK;
	const char* token_start = search;
	while (state != Arcollect::db::search::TOK_EOL) {
		Token next_state = classify_char(search[0]);
		if (next_state == Arcollect::db::search::TOK_INVALID)
			return search;
		if (state != next_state) {
			if (new_token(state,std::string_view(token_start,search-token_start),data))
				return token_start;
			state = next_state;
			token_start = search;
		}
		search++;
	}
	return NULL;
}

struct Arcollect_db_search_do_search_struct {
	Arcollect::db::search::Token last_token = Arcollect::db::search::TOK_BLANK;
	/** Current tag
	 *
	 * This is normally an empty string_view. But in some cases like with
	 * 'black-dragon' tags. Intermediate tokens are parts of the tag.
	 * In such case, 
	 */
	std::string_view current_tag;
	bool current_tag_is_negated = false;
	std::vector<std::string_view> tags;
	std::vector<std::string_view> negated_tags;
};

static bool arcollect_db_search_do_search_callback(Arcollect::db::search::Token token, std::string_view value, void* data)
{
	struct Arcollect_db_search_do_search_struct &search = *reinterpret_cast<struct Arcollect_db_search_do_search_struct*>(data);
	auto last_token = search.last_token;
	search.last_token = token;
	switch (token) {
		case Arcollect::db::search::TOK_NEGATE: {
			switch (last_token) {
				case Arcollect::db::search::TOK_IDENTIFIER: {
					/** We are having a 'black-dragon' like construct and handling the
					 * middle '-'.
					 *
					 * Fake a TOK_IDENTIFIER token for that one.
					 */
					search.last_token = Arcollect::db::search::TOK_IDENTIFIER;
				} return false;
				default: {
					// Skip other tokens
				} return false;
			}
		} return false;
		case Arcollect::db::search::TOK_IDENTIFIER: {
			// Update current_tag
			if (search.current_tag.empty())
				search.current_tag = value;
			else search.current_tag = std::string_view(search.current_tag.data(),search.current_tag.begin()-value.end());
			// last_token dependent handling
			switch (last_token) {
				case Arcollect::db::search::TOK_NEGATE: {
					// '-dragon' like construct -> Raise current_tag_is_negated flag
					search.current_tag_is_negated = true;
				} return false;
				default: {
				} return false;
			}
		} return false;
		case Arcollect::db::search::TOK_BLANK:
		case Arcollect::db::search::TOK_EOL: {
			switch (last_token) {
				case Arcollect::db::search::TOK_IDENTIFIER: {
					// Append the tag
					if (search.current_tag_is_negated)
						search.negated_tags.emplace_back(search.current_tag);
					else search.tags.emplace_back(search.current_tag);
					// Reset current_tag_is_negated flag and current_tag
					search.current_tag_is_negated = false;
					search.current_tag = std::string_view();
				} return false;
				default: return false;
			}
		} return false;
		default: {
		} return false;
	}
}


const char* Arcollect::db::search::build_stmt(const char* search, std::ostream &query, std::vector<std::string_view> &query_bindings)
{
	Arcollect_db_search_do_search_struct src;
	const char* where_its_wrong = Arcollect::db::search::tokenize(search,arcollect_db_search_do_search_callback,&src);
	if (where_its_wrong)
		return where_its_wrong;
	// Manually generate a TOK_EOL
	if (arcollect_db_search_do_search_callback(TOK_EOL,{},&src))
		; // TODO Error reporting
	
	query << "SELECT art_artid,"+Arcollect::db::artid_randomizer+" AS art_order FROM artworks WHERE " << Arcollect::db_filter::get_sql() << " AND (0";
	// Title OR match
	if (src.tags.size() || src.negated_tags.size()) {
		// Note: will be followed by a tag matching
		query << " OR (INSTR(lower(art_title),lower(?)) > 0) OR (1";
		query_bindings.emplace_back(search);
	} else query << " OR 1";// No token: Force a full match
	// Tags OR matching
	if (src.tags.size()) {
		/** Add tag checking logic
		 *
		 * Tags are checked using an EXISTS subquery on art_tag_links JOIN tags.
		 * To implement the AND logic, a LIMIT statement is used to return no row
		 * unless all tags are matched.
		 */
		query <<" AND EXISTS ("
			"SELECT 1 FROM art_tag_links"
			" NATURAL JOIN tags WHERE art_tag_links.art_artid = artworks.art_artid"
			" AND tags.tag_platid in (?";
		query_bindings.emplace_back(src.tags[0]);
		for (decltype(src.tags)::size_type i = 1; i < src.tags.size(); i++) {
			query << ",?";
			query_bindings.emplace_back(src.tags[i]);
		}
		query << ") LIMIT " << (src.tags.size()-1) << ",1)";
	}
	if (src.negated_tags.size()) {
		/** Add tag exclusion logic
		 *
		 * It's the same thing as above but negated and without the LIMIT because a
		 * match inivalidate the whole tag
		 */
		query << " AND NOT EXISTS ("
			"SELECT 1 FROM art_tag_links"
			" NATURAL JOIN tags WHERE art_tag_links.art_artid = artworks.art_artid"
			" AND tags.tag_platid in (?";
		query_bindings.emplace_back(src.negated_tags[0]);
		for (decltype(src.negated_tags)::size_type i = 1; i < src.negated_tags.size(); i++) {
			query << ",?";
			query_bindings.emplace_back(src.negated_tags[i]);
		}
		query << "))";
	}
	if (src.tags.size() || src.negated_tags.size())
		query << ")";
	query << ") ORDER BY art_order;";
	return NULL;
}
bool Arcollect::db::search::build_stmt(const char* search, std::unique_ptr<SQLite3::stmt> &stmt)
{
	std::ostringstream query;
	std::vector<std::string_view> query_bindings;
	if (build_stmt(search,query,query_bindings))
		return true;
	if (database->prepare(query.str().c_str(),stmt))
		return true;
	int i = 1;
	for (auto& binding: query_bindings)
		stmt->bind(i++,binding.data(),binding.size());
	return false;
}
