#include "artwork-collection.hpp"
#include "../db/db.hpp"
namespace Arcollect {
	namespace gui {
		class artwork_collection_simply_all: public artwork_collection {
			// TODO Use the DB.
			public:
				constexpr static const db::artwork_id count = 8;
				class iterator: public iterator_base {
					db::artwork_id counter;
					public:
						iterator& operator++(void) override {
							counter++;
							return *this;
						}
						iterator& operator--(void) override {
							counter--;
							return *this;
						}
						iterator& operator+=(int count) override {
							counter += count+1;
							return *this;
						}
						iterator& operator-=(int count) override {
							counter -= count+1;
							return *this;
						}
						bool operator==(const iterator_base& right) const override {
							return counter == static_cast<const iterator&>(right).counter;
						}
						std::shared_ptr<db::artwork>& operator*(void) override {
							return db::artwork::query(counter);
						};
						iterator(db::artwork_id start_counter = 1) : counter(start_counter) {};
				};
				iterator_base *new_begin(void) override {
					return new iterator();
				}
				iterator_base *new_end(void) override {
					return new iterator(count);
				}
		};
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
				};
				iterator_base *new_begin(void) override {
					return new iterator(artid,false);
				}
				iterator_base *new_end(void) override {
					return new iterator(artid,true);
				}
			artwork_collection_single(db::artwork_id artid) : artid(artid) {};
		};
	}
}
