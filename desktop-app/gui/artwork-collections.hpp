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
#include "artwork-collection.hpp"
#include "../db/db.hpp"
#include <vector>
namespace Arcollect {
	namespace gui {
		/** Artwork collection containing a single artwork
		 */
		class artwork_collection_single: public artwork_collection {
			private:
				db::artwork_id artid;
			public:
				class iterator: public iterator_base {
					db::artwork_id artid;
					bool past_the_end;
					public:
						iterator& operator++(void) override {
							past_the_end = true;
							return *this;
						}
						iterator& operator--(void) override {
							past_the_end = false;
							return *this;
						}
						bool operator==(const iterator_base& right) const override {
							return past_the_end == static_cast<const iterator&>(right).past_the_end;
						}
						std::shared_ptr<db::artwork>& operator*(void) override {
							return db::artwork::query(artid);
						}
						iterator(db::artwork_id artid, bool past_the_end) : artid(artid), past_the_end(past_the_end) {};
						iterator_base* copy(void) override {
							return new iterator(artid,past_the_end);
						}
				};
				iterator_base *new_begin(void) override {
					return new iterator(artid,false);
				}
				iterator_base *new_end(void) override {
					return new iterator(artid,true);
				}
			artwork_collection_single(db::artwork_id artid) : artid(artid) {};
		};
		/** Artwork collection bound to a SQLite query
		 */
		class artwork_collection_sqlite: public artwork_collection {
			private:
				// FIXME Optimize storage and don't buffer the whole transaction uselessly
				std::unique_ptr<SQLite3::stmt> stmt;
				std::vector<db::artwork_id> ids;
			public:
				class iterator: public iterator_base {
					std::vector<db::artwork_id>::iterator iter;
					public:
						iterator& operator++(void) override {
							++iter;
							return *this;
						}
						iterator& operator--(void) override {
							--iter;
							return *this;
						}
						bool operator==(const iterator_base& right) const override {
							return iter == static_cast<const iterator&>(right).iter;
						}
						std::shared_ptr<db::artwork>& operator*(void) override {
							return db::artwork::query(*iter);
						}
						iterator_base* copy(void) override {
							return new iterator(this->iter);
						}
						iterator(std::vector<db::artwork_id>::iterator iter) : iter(iter) {};
				};
				iterator_base *new_begin(void) override {
					return new iterator(ids.begin());
				}
				iterator_base *new_end(void) override {
					return new iterator(ids.end());
				}
			artwork_collection_sqlite(std::unique_ptr<SQLite3::stmt> &in_stmt) :
				stmt(std::move(in_stmt))
			{
				while (stmt->step() == SQLITE_ROW)
					ids.push_back(stmt->column_int64(0));
				stmt.reset();
			}
		};
	}
}
