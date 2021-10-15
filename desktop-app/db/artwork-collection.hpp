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
/** \file artwork-collection.hpp
 *  \brief Artwork listing interface (#Arcollect::db::artwork_collection)
 */
#pragma once
#include "artwork.hpp"
#include "../config.hpp"
namespace Arcollect {
	namespace db {
		/** Artwork listing interface
		 *
		 * An #artwork_collection is an iterable object that yield #db::artworks
		 * pointers.
		 *
		 * GUI parts displayed artwork collection use this.
		 *
		 * \todo Currently, need_entries() is just called in the constructor until
		 *       no more object are available.
		 */
		class artwork_collection {
			private:
				/** Artwork listing cache
				 *
				 * It may be replaced by a more efficient alternative. So it is private.
				 */
				std::vector<artwork_id> cache;
			protected:
				using cache_size_type = decltype(cache)::size_type;
				/** Append a range of new results
				 * \param begin The begin of the range
				 * \param end   The end (excluded) of the ranges
				 * \see Before adding many results, use cache_reserve() if you know the
				 *      size of the result set.
				 */
				void cache_append(const artwork_id* begin, const artwork_id* end) {
					cache.insert(cache.end(),begin,end);
				}
				/** Append a new result
				 * \param id   The id to add
				 * \see Before adding many results, use cache_reserve() if you know the
				 *      size of the result set.
				 */
				void cache_append(artwork_id id) {
					cache.push_back(id);
				}
				/** Reserve space for new results
				 * \param entries_count The number of NEW entries
				 */
				void cache_reserve(cache_size_type entries_count) {
					cache.reserve(cache.size()+entries_count);
				}
				/** Reserve space for new results
				 * \param entries_count The number of entries requested
				 * \return true if there won't be new results
				 *
				 * Within this function, pull new entries and append them in the cache
				 * with cache_append(). You may (and should) pull more than requested.
				 *
				 * The default implementation return true, signaling that no more
				 * entries are available.
				 */
				virtual bool need_entries(cache_size_type entries_count) {
					return true;
				}
				/** find()/find_nearest() assuming "ORDER BY artid_randomizer"
				 *
				 * This is an implementation for and find() and find_nearest() that use
				 * an optimized "divide-and-conquer" search assuming that artworks are
				 * sorted by the #Arcollect::db::artid_randomizer.
				 */
				decltype(cache)::const_iterator find_artid_randomized(artwork_id id, bool nearest);
			public:
				using iterator = decltype(cache)::const_iterator;
				iterator begin(void) {
					return cache.cbegin();
				}
				iterator end(void) {
					return cache.cend();
				}
				/** Find the iterator to an artwork
				 * \param id The artwork to find
				 * \return An iterator to the artwork or end() if not found
				 * 
				 * The default implementation perform a naive full travel.
				 * The collection may override this function with a more efficient
				 * algorithm.
				 */
				virtual iterator find(artwork_id id);
				/** Find the iterator to an artwork
				 * \param artwork The artwork to find
				 * \return An iterator to the artwork or end() if not found
				 * 
				 * The default implementation perform a naive full travel.
				 * The collection may override this function with a more efficient
				 * algorithm.
				 */
				iterator find(const std::shared_ptr<db::artwork> &artwork) {
					return find(artwork->art_id);
				}
				/** Find the nearest iterator to an artwork
				 * \param id The artwork to find
				 * \return An iterator near the artwork, may return end()
				 * 
				 * The default implementation just call find(). The collection may
				 * override this function with an efficient near finding algorithm.
				 */
				virtual iterator find_nearest(artwork_id id) {
					return find(id);
				}
				/** Find the nearest iterator to an artwork
				 * \param artwork The artwork to find
				 * \return An iterator near the artwork, may return end()
				 * 
				 * The default implementation just call find(). The collection may
				 * override this function with an efficient near finding algorithm.
				 */
				iterator find_nearest(const std::shared_ptr<artwork> &artwork) {
					return find_nearest(artwork->art_id);
				}
				artwork_collection(void);
				virtual ~artwork_collection(void) = default;
				
				/** Delete all artworks in this collection
				 *
				 * This is not an innocent function !
				 */
				int db_delete(void);
				/** Set rating of all artworks in this collection
				 */
				int db_set_rating(Arcollect::config::Rating rating);
		};
	}
}
