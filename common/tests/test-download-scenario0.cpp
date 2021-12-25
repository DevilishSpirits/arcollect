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
#include "arcollect-db-open.hpp"
#include "arcollect-db-downloads.hpp"
#include <iostream>
using Arcollect::db::downloads::DownloadInfo;
using Arcollect::db::downloads::Transaction;

static int test_num = 1;
static decltype(std::cout)& ok_nok(bool result, bool assert_passing)
{
	return std::cout << (result == assert_passing ? "ok " : "not ok ") << test_num++ << " - ";
}

static void assert_write_cache(bool assert_passing, const std::string_view &name, const std::string_view &db_key, const std::string_view &mimetype, DownloadInfo& infos, Transaction &cache)
{
	std::string res = cache.write_cache(db_key,mimetype,infos);
	ok_nok(res.empty(),assert_passing) << name;
	if (!res.empty())
		std::cout << ": " << res;
	std::cout << std::endl;
}

struct Entry {
	std::string db_key;
	std::string mimetype;
	DownloadInfo infos;
	Transaction &cache;
	bool operator==(const Entry& right) const {
		return 
			(db_key == right.db_key)&&
			(mimetype == right.mimetype)&&
			(infos.dwn_lastedit == right.infos.dwn_lastedit)&&
			(infos.dwn_etag == right.infos.dwn_etag)&&
			(infos.dwn_path == right.infos.dwn_path);
	}
	Entry(Transaction &cache) : cache(cache) {}
	Entry(const Entry&) = default;
	
	void assert_write_cache(bool assert_passing, const std::string_view &name) {
		return ::assert_write_cache(assert_passing,name,db_key,mimetype,infos,cache);
	}
	void assert_equal(bool assert_passing, const std::string_view &name, const Entry& right) {
		ok_nok(assert_passing,operator==(right)) << name << std::endl;
	}
};

int main(void)
{
	std::cout << "1..9" << std::endl;
	std::unique_ptr<SQLite3::sqlite3> database = Arcollect::db::test_open();
	Transaction cache(database);
	// 1. Add initial set
	Entry entry1(cache);
	entry1.db_key = "https://example.com/random.png";
	entry1.mimetype = "image/*";
	entry1.infos.dwn_lastedit = 0;
	entry1.infos.dwn_etag = "123abc";
	entry1.infos.dwn_path = "test";
	
	// 2. Add a second set and ensure it changes
	Entry entry2(entry1);
	
	entry1.assert_write_cache(true,"Creating first set");
	entry1.assert_equal(true,"Ensure the first set has not been altered",entry2);
	
	entry2.db_key = "456def";
	entry2.assert_write_cache(true,"Creating second set");
	entry1.assert_equal(false,"Ensure the second set is different of the first set",entry2);
	
	// 3. Remove the first set and ensure it filename is not reuse
	Entry entry3(entry1);
	cache.delete_cache(entry1.infos.dwn_id());
	entry3.assert_write_cache(true,"Reinserting first set after deleting it");
	entry1.assert_equal(false,"Ensure the reinserted first set is different from the first set",entry3);
	
	// 4. Assert write_cache success on same db_key with a different dwn_id
	Entry entry4(entry2);
	auto query1 = cache.query_cache(entry2.db_key);
	entry2.assert_write_cache(true,"Reinsert second set");
	ok_nok(query1.dwn_id() != entry2.infos.dwn_id(),true) << "Ensure that the dwn_id of second set has been updated" << std::endl;
	
	// 5. Assert that unsource works on entry2
	cache.unsource(entry2.infos.dwn_id());
	auto query2 = cache.query_cache(entry2.db_key);
	ok_nok(query2,false) << "Ensure that the unsource of second set worked" << std::endl;
	
	database->exec("COMMIT;");
}
