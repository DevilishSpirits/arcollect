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
/** \file desktop-app/db/sorting.hpp
 *  \brief Artwork sorting
 */
#pragma once
#include <ctime>
#include <string_view>
#include "search.hpp"
namespace Arcollect {
	namespace db {
		class artwork;
		/** A sorting implementation
		 *
		 * This structure contain everything Arcollect to implement an artwork
		 * sorting algo.
		 */
		struct SortingImpl {
			/** Artwork comparison C++ function
			 * \param left  The left artwork.
			 * \param right The right artwork.
			 * \return If `left < right`.
			 */
			bool(*compare_arts)(const artwork& left, const artwork& right);
			/** SELECT SQL additional statement
			 *
			 * This string must be added after the SELECT column list
			 */
			const std::string_view(*sql_select_trailer)(SearchType search_type);
			/** SQL ORDER BY clause
			 */
			const std::string_view(*sql_order_by)(SearchType search_type);
			
			bool compare(const artwork& left, const artwork& right) {
				return compare_arts(left,right);
			}
		};
		/** Get sorting implementation by mode
		 */
		const SortingImpl& sorting(SortingType mode);
		/** Apply #Arcollect::db::artid_randomizer
		 * \todo No longer expose this in public API
		 */
		sqlite_int64 artid_randomize(sqlite_int64 art_artid);
	}
}
