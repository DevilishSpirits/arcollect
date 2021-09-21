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
#include "sorting.hpp"
const std::time_t Arcollect::db::artid_randomizer_seed = std::time(NULL);

static const Arcollect::db::sorting::Implementation sorting_impl_random = {
	[](const Arcollect::db::artwork& left, const Arcollect::db::artwork& right) -> bool {
		return Arcollect::db::artid_randomize(left.art_id) < Arcollect::db::artid_randomize(right.art_id);
	},
	",((art_artid+"+std::to_string(Arcollect::db::artid_randomizer_seed)+")*2654435761) % 4294967296 AS art_order"
	" FROM artworks",
	"ORDER BY art_order;"
};

/** Get implementation by mode
*/
const Arcollect::db::sorting::Implementation& Arcollect::db::sorting::implementations(Arcollect::db::sorting::Mode mode)
{
	switch (mode) {
		case Arcollect::db::sorting::Mode::RANDOM:return sorting_impl_random;
		default:return sorting_impl_random;
	}
}
