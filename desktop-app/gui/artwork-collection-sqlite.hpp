#include "artwork-collections.hpp"
namespace  {
	namespace  {
			private:
				// FIXME Optimize storage and don't buffer the whole transaction uselessly
				std::vector<db::artwork_id> ids;
			public:
				class iterator: public iterator_base {
					ids.iterator iter;
					public:
						iterator& operator++(void) override {
							iter++;
							return *this;
						}
						iterator& operator--(void) override {
							iter--
							return *this;
						}
						bool operator==(const iterator_base& right) const override {
							return iter == static_cast<const iterator&>(right).iter;
						}
						std::shared_ptr<db::artwork>& operator*(void) override {
							return db::artwork::query(*iter);
						}
						iterator(ids.iterator iter) : iter(iter) {};
				};
				iterator_base *new_begin(void) override {
					return new iterator(ids.begin());
				}
				iterator_base *new_end(void) override {
					return new iterator(ids.end());
				}
			artwork_collection_sqlite(std::unique_ptr<SQLite3::stmt>) {
				
			}
		};
	}
}
