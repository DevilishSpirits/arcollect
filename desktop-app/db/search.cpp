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
#include "sorting.hpp"
#include "artwork-collections.hpp"
#include "db.hpp"
#include "../config.hpp"
#include "../gui/font.hpp"
#include <arcollect-debug.hpp>
#include <functional>
#include <map>
#include <type_traits>
#include <unordered_set>
using Arcollect::db::SearchType;
using Arcollect::search::ParsedSearch;
static Arcollect::config::Rating parse_rating(const std::string_view& value) {
	if (value.size())
		switch (value[0]) {
			case 'e':case 'E':
				return Arcollect::config::RATING_ADULT;
			case 'q':case 'Q':
				return Arcollect::config::RATING_MATURE;
			case 's':case 'S':
				return Arcollect::config::RATING_NONE;
		}
	return Arcollect::config::RATING_NONE;
}
static SDL::Color make_rating_color(Arcollect::config::Rating rating) {
	return (rating <= Arcollect::config::RATING_NONE) ? SDL::Color{0,255,0,255} :
		(rating <= Arcollect::config::RATING_MATURE) ? SDL::Color{0,0,255,255} :
		SDL::Color{255,0,0,255};
}
/** Set of tags
 *
 * A tag set hold a list of tags and their negated tags. For example
 * `dragon -noodle` is a set of 1 positive and 1 negative tag.
 *
 * Arcollect also use this structure for `account:abc` objects to make
 * things easier.
 */
template <typename T>
struct TagSet {
	/** Container of matchs typedef
	 */
	using matchs_container = std::unordered_set<T>;
	/** Positive matchs
	 */
	matchs_container positive_matchs;
	/** Negative matchs
	 */
	matchs_container negative_matchs;
	/** Select positive or negated matchs list
	 * \param negated Wheather to return the negated match list
	 * \return positive_matchs if negated is `false`, negative_matchs
	 *         otherwhise.
	 */
	matchs_container &operator[](bool negated) {
		return negated ? negative_matchs : positive_matchs;
	}
	static void generate_matchs_sql_list(const matchs_container& container, std::string &query, ParsedSearch::sql_bindings_type &bindings) {
		// Generate SQL placeholders
		query += "?";
		for (typename matchs_container::size_type i = 1; i < container.size(); i++)
			query += ",?";
		// Add bindings
		for (const T& match: container)
			bindings.push_back(match);
	}
	/** Generate the SQL WHERE statement that match in a links table
	 *
	 */
	void gen_link_matching_sql(const std::string_view &source_table, const std::string_view &link_table, const std::string_view &linked_table, const std::string_view &link_idcol, const std::vector<std::string_view> &cols_match, std::string &query, ParsedSearch::sql_bindings_type &bindings) const {
		if (!positive_matchs.empty()) {
			query += "AND EXISTS (SELECT 1 FROM ";
			query += link_table;
			query += " NATURAL JOIN ";
			query += linked_table;
			query += " WHERE ";
			query += link_table;
			query += ".";
			query += link_idcol;
			query += " = ";
			query += source_table;
			query += ".";
			query += link_idcol;
			query += " AND (0";
				for (const std::string_view& col: cols_match) {
					query += " OR(";
					query += linked_table;
					query += ".";
					query += col;
					query += " IN(";
					generate_matchs_sql_list(positive_matchs,query,bindings);
					query += "))";
				}
				query += ") LIMIT " + std::to_string(positive_matchs.size()-1) + ",1)";
		}
		if (!negative_matchs.empty()) {
			query += "AND NOT EXISTS (SELECT 1 FROM ";
			query += link_table;
			query += " NATURAL JOIN ";;
			query += linked_table;
			query += " WHERE ";
			query += link_table;
			query += ".";
			query += link_idcol;
			query += " = ";
			query += source_table;
			query += ".";
			query += link_idcol;
			query += " AND(0 ";
				for (const std::string_view& col: cols_match) {
					query += "OR(";
					query += linked_table;
					query += ".";
					query += col;
					query += " IN(";
					generate_matchs_sql_list(negative_matchs,query,bindings);
					query += "))";
				}
				query += ")LIMIT 1)";
		}
	}
	void gen_exact_matching_sql(const std::string_view &table_dot_col, std::string &query, ParsedSearch::sql_bindings_type &bindings) const {
		for (const auto &match: positive_matchs) {
			query += " AND(";
			query += table_dot_col;
			query += " = ?)";
			bindings.push_back(match);
		}
		for (const auto &match: negative_matchs) {
			query += " AND(";
			query += table_dot_col;
			query += " != ?)";
			bindings.push_back(match);
		}
	}
	void gen_prefix_matching_sql(const std::string_view &table_dot_col, std::string &query, ParsedSearch::sql_bindings_type &bindings) const {
		for (const std::string_view &match: positive_matchs) {
			query += "AND arcollect_match_prefix(";
			query += table_dot_col;
			query += ",?)";
			bindings.push_back(match);
		}
		for (const std::string_view &match: negative_matchs) {
			query += "AND NOT arcollect_match_prefix(";
			query += table_dot_col;
			query += ",?)";
			bindings.push_back(match);
		}
	}
	std::function<void(const std::string_view,bool)> tokenize_func(void) {
		return [this](const std::string_view data,bool negated){
			operator[](negated).emplace(T(data));
		};
	}
};

static void tokenize(const std::string_view& search, const std::unordered_map<std::string_view,std::function<void(std::string_view,bool)>> &output)
{
	enum {
		BLANK,
		VERB,
		VALUE,
	} parse_stage = BLANK;
	auto verb_iter = output.find("");
	bool negated = false;
	const char* start = NULL;
	for (const char &chr: search)
		switch (parse_stage) {
			case BLANK: {
				start = &chr;
				if (chr == '-')
					negated = true;
				else if (!std::isblank(chr)) {
					parse_stage = VERB;
					start = &chr;
				} else negated = false;
			} break;
			case VERB: {
				if (chr == ':') {
					verb_iter = output.find(std::string_view(start,std::distance(start,&chr)));
					start = NULL;
					parse_stage = VALUE;
					break;
				}
			} // falltrough;
			case VALUE: {
				if (start == NULL)
					start = &chr;
				if (std::isblank(chr)) {
					if (verb_iter != output.cend())
						verb_iter->second(std::string_view(start,std::distance(start,&chr)),negated);
					verb_iter = output.cend();
					parse_stage = BLANK;
				}
			} break;
		}
	// Add the last word
	if (start)
		switch (parse_stage) {
			case VERB: {
				verb_iter = output.find("");
			} // falltrough;
			case VALUE: {
					if (verb_iter != output.cend())
						verb_iter->second(std::string_view(start,std::distance(start,search.end())),negated);
			} break;
			case BLANK: {
				// Do nothing
			} break;
		}
}

template <const char* string>
struct SQLDontPrintOnce {
	bool do_print = false;
};
template <const char* string>
std::string &operator+=(std::string &left, SQLDontPrintOnce<string> &right) {
	if (right.do_print)
		left += string;
	else right.do_print = true;
	return left;
}
static constexpr char sql_and[] = " AND ";
static constexpr char sql_or[]  = " OR ";
using SQLAnd = SQLDontPrintOnce<sql_and>;
using SQLOr = SQLDontPrintOnce<sql_or>;
template<class> inline constexpr bool always_false_v = false;
/** Set of match filters
 *
 * This object store a set of criterias to AND.
 */
struct MatchExpr {
	TagSet<std::string_view> tags;
	TagSet<std::string_view> accounts;
	TagSet<std::string_view> platforms;
	TagSet<sqlite_int64> ratings;
	TagSet<std::string_view> ratings_string;
	std::vector<std::unique_ptr<MatchExpr>> or_subexprs;
	
	void gen_artworks_sql(std::string &query, ParsedSearch::sql_bindings_type &bindings) const {
		tags.gen_link_matching_sql("artworks","art_tag_links","tags","art_artid",{"tag_platid","tag_title"},query,bindings);
		accounts.gen_link_matching_sql("artworks","art_acc_links","accounts","art_artid",{"acc_platid","acc_name","acc_title"},query,bindings);
		platforms.gen_prefix_matching_sql("artworks.art_platform",query,bindings);
		ratings.gen_exact_matching_sql("artworks.art_rating",query,bindings);
		
		if (!or_subexprs.empty()) {
			SQLOr _or_;
			query += " AND (";
			for (const std::unique_ptr<MatchExpr> &subexpr: or_subexprs) {
				query += _or_;
				query += "(";
				subexpr->gen_artworks_sql(query,bindings);
				query += ")";
			}
			query += ")";
		}
	}
	
	struct our_std_less {
		constexpr bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
			return std::less<std::string_view::const_pointer>()(lhs.data(),rhs.data());
		}
	};
	/** Search coloration container type
	 *
	 * This map hold a list of std::string_view and the color to use.
	 *
	 * MatchExpr doesn't preserve the order of tokens while parsing the
	 * expression but it's std::string_view reference the search string that is
	 * naturally ordered. Map keys is sorted with ascending start in the string,
	 * this allow easy search coloration by colorize_search().
	 */
	using text_colors_maps = std::map<std::string_view,SDL::Color,our_std_less>;
	
	constexpr static SDL::Color positive_matchs_color{192,255,192,255};
	constexpr static SDL::Color negative_matchs_color{255,192,192,255};
	/** Generate text colors for a tagset
	 * \param tagset to colorize
	 * \param verb_length is the length of the prefix verb like `"account"` for
	 *                    *account* (note that the `':'` is included).
	 *                    You should use `sizeof("account")` values.
	 * \param verb_color is the color for the verb.
	 *
	 * \warning A too large `verb_length` might trigger buffer underflows.
	 */
	template <typename TagsetT>
	static void gen_text_colors_for_tagset(const TagsetT &tagset, text_colors_maps &colors, int verb_length, SDL::Color verb_color) {
		for (const auto& match: tagset.positive_matchs) {
			colors.emplace(std::string_view(match.data()-verb_length,verb_length-1),verb_color);
			colors.emplace(match,positive_matchs_color);
		}
		for (const auto& match: tagset.negative_matchs) {
			// FIXME colors.emplace(std::string_view(match.data()-verb_length-1,1),SDL::Color{255,0,0,255});
			colors.emplace(std::string_view(match.data()-verb_length,verb_length-1),verb_color);
			colors.emplace(match,negative_matchs_color);
		}
	}
	
	/** Generate text coloration
	 * \param[out] colors Output of color data
	 *
	 * You can generate Arcollect::gui::font::Elements with colorize_search().
	 *
	 * This variant exists for internal operation, you can use gen_text_colors()
	 * that directly returns a #text_colors_maps.
	 */
	void gen_artworks_text_colors(text_colors_maps &colors) const {
		for (const std::string_view& match: tags.negative_matchs) {
			// FIXME colors.emplace(std::string_view(match.data()-1,1),SDL::Color{255,0,0,255});
			colors.emplace(match,negative_matchs_color);
		}
		
		gen_text_colors_for_tagset(accounts,colors,sizeof("account"),{0,255,0,255});
		gen_text_colors_for_tagset(platforms,colors,sizeof("site"),{0,0,255,255});
		
		for (const auto& match: ratings_string.positive_matchs) {
			colors.emplace(std::string_view(match.data()-7,6),SDL::Color{128,128,128,255});
			colors.emplace(match,make_rating_color(parse_rating(match)));
		}
		for (const auto& match: ratings_string.negative_matchs) {
			// FIXME colors.emplace(std::string_view(match.data()-8,1),SDL::Color{255,0,0,255});
			colors.emplace(std::string_view(match.data()-7,6),SDL::Color{128,128,128,255});
			colors.emplace(match,make_rating_color(parse_rating(match)));
		}
		
		for (const std::unique_ptr<MatchExpr>& subexpr: or_subexprs)
			subexpr->gen_artworks_text_colors(colors);
	}
	/** Generate text coloration
	 * \return Output of color data
	 *
	 * You can generate Arcollect::gui::font::Elements with colorize_search().
	 */
	text_colors_maps gen_text_colors(SearchType search_type) const {
		text_colors_maps result;
		switch (search_type) {
			case Arcollect::db::SEARCH_ARTWORKS: {
				gen_artworks_text_colors(result);
			} break;
		}
		return result;
	}
	static Arcollect::gui::font::Elements colorize_search(const std::string_view& search, const text_colors_maps& map) {
		Arcollect::gui::font::Elements elements;
		const char* iter = search.data();
		for (const auto &pair: map) {
			// Print elements between two colored set in white (color is already white)
			elements << std::string_view(iter,std::distance(iter,pair.first.data()))
			// Print the colored string
			         << pair.second << pair.first
			// Reset to white
			         << SDL::Color(255,255,255,255);
			// Place the cursor at the end of the colored area
			iter = &*pair.first.end();
		}
		// Print the rest of the string
		return elements << std::string_view(iter,search.size()-std::distance(search.data(),iter));
	}
};


Arcollect::search::ParsedSearch::ParsedSearch(std::string &&search_terms, SearchType search_type, SortingType sorting_type) :
	search(std::move(search_terms)),
	search_type(search_type),
	real_sorting_type(sorting_type)
{
	if (Arcollect::debug.search)
		std::cerr << "Query:" << search_terms << std::endl;
	// Parse the expression
	MatchExpr expr;
	sql_query.reserve(65536); // Preallocate a large chunk of memory
	const std::unordered_map<std::string_view,std::function<void(std::string_view,bool)>> tagset_map = {
		{"",expr.tags.tokenize_func()},
		{"account",expr.accounts.tokenize_func()},
		{"site",expr.platforms.tokenize_func()},
		{"rating",[&](const std::string_view data,bool negated){
			expr.ratings[negated].emplace(parse_rating(data));
			expr.ratings_string[negated].emplace(data);
		}},
		{"sort",[&](const std::string_view data,bool negated) {
			if (data.starts_with("r"))
				real_sorting_type = Arcollect::db::SORT_RANDOM;
			else if (data.starts_with("s"))
				real_sorting_type = Arcollect::db::SORT_SAVEDATE;
		}}
	};
	tokenize(search,tagset_map);
	sql_query = "SELECT art_artid";
	sql_query += sorting().sql_select_trailer(search_type);
	sql_query += " FROM artworks WHERE ((1 ";
	const auto sql_query_size = sql_query.size();
	expr.gen_artworks_sql(sql_query,sql_bindings);
	if (sql_query_size == sql_query.size())
		sql_query += "AND 1 ";
	sql_query += ")OR(INSTR(lower(art_title),lower(?)) > 0)) AND art_rating <= ? ";
	sql_query += sorting().sql_order_by(search_type);
	sql_query += ";";
	sql_bindings.push_back(search);
	sql_query.shrink_to_fit();
	if (Arcollect::debug.search) {
		std::cerr << "\tSQL:" << sql_query << "\n\tSQL bindings:";
		for (const auto& binding: sql_bindings) {
			std::visit([](auto&& binding) {
				std::cerr << "\n\t\t" << binding;
				using T = std::decay_t<decltype(binding)>;
				if constexpr (std::is_same_v<T, std::string_view>)
					std::cerr << "\tTEXT";
				else if constexpr (std::is_same_v<T, sqlite_int64>)
					std::cerr << "\tINTEGER";
				else static_assert(always_false_v<T>, "non-exhaustive visitor!");
			}, binding);
		}
		std::cerr << std::endl;
	}
	// Generate the text
	cached_elements = std::make_unique<Arcollect::gui::font::Elements>(expr.colorize_search(search,expr.gen_text_colors(search_type)));
}

void Arcollect::search::ParsedSearch::build_stmt(std::unique_ptr<SQLite3::stmt> &stmt) const
{
	if (database->prepare(sql_query.data(),stmt))
		std::cerr << "Search SQL prepare failure: " << database->errmsg() << " Search was: \"" << search << "\". Query was " << sql_query << "std::endl";
	int i = 1;
	for (const auto& binding: sql_bindings)
		std::visit([&](auto&& binding) {
			stmt->bind(i++,binding);
		}, binding);
	stmt->bind(i++,Arcollect::config::current_rating);
}
std::shared_ptr<Arcollect::db::artwork_collection> Arcollect::search::ParsedSearch::make_shared_collection(void) const
{
	std::unique_ptr<SQLite3::stmt> stmt;
	build_stmt(stmt);
	std::shared_ptr<Arcollect::db::artwork_collection> result = std::make_shared<Arcollect::db::artwork_collection_sqlite>(std::move(stmt));
	result->sorting_type = sorting_type();
	return result;
}
