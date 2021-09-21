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
#include "artwork.hpp"
#include <ctime>
#include <functional>
#include <string>
namespace Arcollect {
	namespace db {
		namespace sorting {
			/** Ordering mode
			 */
			enum Mode {
				/** Original stable random ordering (default)
				 *
				 * This algo is inspired from the Knuth's multiplicative method hash with
				 * the addition of the well-know "random" quantity `time(NULL)` to simulate
				 * randomness.
				 *
				 * It avoid a complete reorganization of the slideshow grid with random
				 * sort when rerunning an SQL statementsss.
				 *
				 * This generate a value to "ORDER BY" against that is computed by
				 * '((art_artid+time(NULL))*2654435761) % 4294967296' and should works
				 * for most collections.
				 */
				RANDOM,
			};
			/** A sorting implementation
			 *
			 * This structure contain everything Arcollect to implement an artwork
			 * sorting algo.
			 */
			struct Implementation {
				/** Artwork comparison C++ function
				 * \param left  The left artwork.
				 * \param right The right artwork.
				 * \return If `left < right`.
				 */
				std::function<bool(const artwork& left, const artwork& right)> compare_arts;
				/** SELECT SQL additional statement and FROM
				 *
				 * This string must be added after the SELECT column list and include 
				 * the FROM statement
				 */
				std::string sql_select_and_from;
				/** SQL query trailer (ORDER BY)
				 *
				 * This string must be added after the SQL query.
				 * It does include the trailing semi-colon `;`.
				 */
				std::string sql_trailer;
			};
			/** Get implementation by mode
			 */
			const Implementation& implementations(Mode mode);
		}
		/** Seed used for #Arcollect::db::artid_randomizer
		 *
		 * It's set to std::time(NULL) at program init.
		 * \todo No longer expose this in public API
		 */
		extern const std::time_t artid_randomizer_seed;
		/** Apply #Arcollect::db::artid_randomizer
		 * \todo No longer expose this in public API
		 */
		inline sqlite_int64 artid_randomize(sqlite_int64 art_artid) {
			return ((art_artid+artid_randomizer_seed)*2654435761) % 4294967296;
		}
	}
}
