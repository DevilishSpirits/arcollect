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
#include "artwork.hpp"
using Arcollect::db::SearchType;
using Arcollect::db::SortingType;
static std::time_t artid_randomizer_seed(void) {
	static std::time_t value = std::time(NULL);
	return value;
}
sqlite_int64 Arcollect::db::artid_randomize(sqlite_int64 art_artid) {
	return ((art_artid+artid_randomizer_seed())*2654435761) % 4294967296;
}

static std::string empty_string = "";
static const std::string_view nothing(SearchType search_type)
{
	return empty_string;
}
static const Arcollect::db::SortingImpl sorting_impl_none = {
	[](const Arcollect::db::artwork& left, const Arcollect::db::artwork& right) -> bool {
		return false;
	},nothing,nothing,
};
static const Arcollect::db::SortingImpl sorting_impl_random = {
	[](const Arcollect::db::artwork& left, const Arcollect::db::artwork& right) -> bool {
		if (Arcollect::db::artid_randomize(left.partof()) != Arcollect::db::artid_randomize(right.partof()))
			return Arcollect::db::artid_randomize(left.partof()) < Arcollect::db::artid_randomize(right.partof());
		else if (left.pageno() != right.pageno())
			return left.pageno() < right.pageno();
		else return left.art_id < right.art_id;
	},
	[](SearchType search_type) -> const std::string_view {
		switch (search_type) {
			default:
			case Arcollect::db::SEARCH_ARTWORKS: {
				static std::string result = ",((art_partof+"+std::to_string(artid_randomizer_seed())+")*2654435761) % 4294967296 AS art_order";
				return result;
			} break;
		}
	},
	[](SearchType search_type) -> const std::string_view {
		switch (search_type) {
			default:
			case Arcollect::db::SEARCH_ARTWORKS: {
				return "ORDER BY art_order, art_pageno, art_artid";
			} break;
		}
	},
};
static const Arcollect::db::SortingImpl sorting_impl_savedate = {
	[](const Arcollect::db::artwork& left, const Arcollect::db::artwork& right) -> bool {
		return left.savedate() < right.savedate();
	},nothing,
	[](SearchType search_type) -> const std::string_view {
		switch (search_type) {
			default:
			case Arcollect::db::SEARCH_ARTWORKS: {
				return "ORDER BY art_savedate, art_artid";
			} break;
		}
	},
};

/** Get implementation by mode
*/
const Arcollect::db::SortingImpl& Arcollect::db::sorting(SortingType mode)
{
	switch (mode) {
		case SORT_NONE:return sorting_impl_none;
		case SORT_RANDOM:return sorting_impl_random;
		case SORT_SAVEDATE:return sorting_impl_savedate;
	}
	return sorting_impl_random;
}
