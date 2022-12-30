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
#include <arcollect-db-open.hpp>
#include "../db/db.hpp"
#include "../db/sorting.hpp"
#include <iostream>
#include "../sdl2-hpp/SDL.hpp" // For SDL_main

using Arcollect::db::SearchType;
using Arcollect::db::SortingType;
using Arcollect::search::ParsedSearch;

static int failed = false;

constexpr Arcollect::db::SortingType sorting_types[] = {Arcollect::db::SORT_NONE,Arcollect::db::SORT_RANDOM};

std::ostream& operator<<(std::ostream& lhs, SearchType rhs) {
	switch (rhs) {
		case Arcollect::db::SEARCH_ARTWORKS:return lhs << "SEARCH_ARTWORKS";
		default:failed = true;return lhs << "SEARCH_\?\?\?(" << static_cast<int>(rhs) << ")";
	}
}
std::ostream& operator<<(std::ostream& lhs, SortingType rhs) {
	switch (rhs) {
		case Arcollect::db::SORT_NONE:return lhs << "SORT_NONE";
		case Arcollect::db::SORT_RANDOM:return lhs << "SORT_RANDOM";
		default:failed = true;return lhs << "SORT_\?\?\?(" << static_cast<int>(rhs) << ")";
	}
}
static int test_num = 1;
static void test_parsed_search(const ParsedSearch& search) {
	std::unique_ptr<SQLite3::stmt> stmt;
	search.build_stmt(stmt);
	if (!stmt) {
		std::cout << "not ";
		failed = true;
	}
	std::cout << "ok " << test_num++ << " - ParsedSearch(\"" << search.search << "\"," << search.search_type << "," << search.sorting_type() << ")" << std::endl;
}
static constexpr auto test_num_per_test_parsed_search = 1;

static void test_artworks_search_expression(const std::string_view& search) {
	std::cout << "# Testing \"" << search << "\" artworks search" << std::endl;
	for (SortingType sorting_type: sorting_types)
		test_parsed_search(ParsedSearch(search,Arcollect::db::SEARCH_ARTWORKS,sorting_type));
}
static constexpr auto test_num_per_test_artworks_search_expression = test_num_per_test_parsed_search*sizeof(sorting_types)/sizeof(sorting_types[0]);

static constexpr std::string_view search_exprs[] = {
	// Original set from the now defunct test-search-tokenizer.cpp
	"dragon",
	"-dragon",
	"-black-dragon",
	"bat digital-2d",
	"dragon site:artstation.com",
	
	// Some random ideas from a lack of imagination
	"-imagination",
	"account:DevilishSpirits",
	"Your personal visual artworks library",
	"bad-dragon rating:s",
	
	"Twentie-oneth release (v0.23)",
	"Twentieth release (v0.22)",
	" Comics and nineteenth release (v0.21)",
	"Comics bêta and eighteenth release (v0.20)",
	"I18n and seventeenth release (v0.19)",
	"Sixteenth release (v0.18)",
	"Fifthteenth release (v0.17)",
	"Twitter support and fourteenth release (v0.16)",
	"Thirteenth release (v0.15)",
	"Stories, font and twelfth release (v0.14)",
	"Eleventh release (v0.13)",
	"Tenth release (v0.12)",
	"Color management and ninth release (v0.11)",
	"Windows 10 port and eighth release (v0.10)",
	"Seventh release (v0.9)",
	"Sixth release (v0.8)",
	"Fifth release (v0.7)",
	"Fourth release (v0.6)",
	"Third release (v0.5)",
	"Second release (v0.4)",
	"First public release (v0.3)",
};

int main(int argc, char *argv[])
{
	Arcollect::database = Arcollect::db::test_open();
	std::cout << "TAP version 13\n1.." << (
		test_num_per_test_artworks_search_expression * (sizeof(search_exprs)/sizeof(search_exprs[0]))
	) << std::endl;
	for (const std::string_view& search: search_exprs)
		test_artworks_search_expression(search);
	return failed;
}
