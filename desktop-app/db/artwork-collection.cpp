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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "artwork-collection.hpp"
#include "db.hpp"
Arcollect::db::artwork_collection::artwork_collection(void)
{
	while (!need_entries(4096));
	cache.shrink_to_fit();
}
Arcollect::db::artwork_collection::iterator Arcollect::db::artwork_collection::find(artwork_id id)
{
	iterator iter = begin();
	const iterator iter_end = end();
	for (; (iter != iter_end) && (*iter != id); ++iter);
	return iter;
}

 Arcollect::db::artwork_collection::iterator Arcollect::db::artwork_collection::find_artid_randomized(artwork_id id, bool nearest)
{
	cache_size_type left = 0;
	cache_size_type size = cache.size();
	auto target = db::artid_randomize(id);
	while (size) {
		size -= 1;
		size /= 2;
		auto current = left + size;
		auto diff = db::artid_randomize(cache[current])-target;
		if (diff == 0)
			// Match
			return begin()+current;
		else if (diff < 0)
			// Match is in the right part, update left
			left += size + 1;
		//else size will be shrink on next loop iteration, nothing to do
	}
	if (nearest)
		return begin()+left;
	else return end();
}
