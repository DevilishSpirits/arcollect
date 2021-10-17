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
/** \file artwork-collections.hpp
 *  \brief Common implementations of #Arcollect::db::artwork_collection.
 */
#pragma once
#include "artwork-collection.hpp"
#include "../db/db.hpp"
namespace Arcollect {
	namespace db {
		/** Artwork collection bound to a SQLite query
		 *
		 * \warning The results must be ordered by "ORDER BY artid_randomizer" as
		 *          defined by #Arcollect::db::artid_randomizer.
		 */
		class artwork_collection_sqlite: public artwork_collection {
			public:
				/** Constructor
				 * \param stmt The prepared stmts
				 *
				 * The statement must yield the art_artid and it will be stealed.
				 */
				artwork_collection_sqlite(std::unique_ptr<SQLite3::stmt> &&stmt)
				{
					while (stmt->step() == SQLITE_ROW)
						cache_append(stmt->column_int64(0));
					stmt.reset();
				}
				artwork_collection::iterator find(artwork_id id) override {
					return find_artid_randomized(id,false);
				}
				artwork_collection::iterator find_nearest(artwork_id id) override {
					return find_artid_randomized(id,true);
				}
		};
		/** Artwork collection bound to a single artwork
		 */
		class artwork_collection_single: public artwork_collection {
			public:
				/** Constructor
				 * \param id The artwork id
				 */
				artwork_collection_single(artwork_id id)
				{
					cache_append(id);
				}
		};
	}
}
