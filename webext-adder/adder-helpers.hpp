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
/** \file webext-adder/adder-helpers.hpp
 *  \brief adder.cpp helpers types
 *
 * This file implement contains many helpers types for adder.cpp, they are
 * separated to shorten it.
 */
template <typename KeyT>
struct default_SQLBinder {
	static int bind(SQLite3::stmt &stmt, const KeyT& key, const std::string_view& platform) {
		return stmt.bind(1,key);
	}
};

template <typename KeyT,typename DBCacheT>
struct JSONRefReader {
};

template<typename DBCacheT>
struct JSONRefReader<std::string_view,DBCacheT> {
	static typename DBCacheT::key_type read_ref(Arcollect::json::ObjHave have, const char* errmsg, char*& iter, char* const end) {
		std::string_view key;
		json_read_string(have,key,errmsg,iter,end);
		return key;
	}
};
template<typename DBCacheT>
struct JSONRefReader<platform_id,DBCacheT> {
	static typename DBCacheT::key_type read_ref(Arcollect::json::ObjHave have, const char* errmsg, char*& iter, char* const end) {
		platform_id platid;
		read_platform_id(have,platid,errmsg,iter,end);
		return platid;
	}
};

template <typename ValueT>
struct DBCacheOptionalBase {
	std::optional<ValueT> opt;
	operator std::optional<ValueT>&(void) {
		return opt;
	}
	ValueT* operator->(void) {
		return opt.operator->();
	}
	template <class... Args>
	ValueT& emplace(Args&&... args) {
		return opt.emplace(std::forward<Args>(args)...);
	}
	operator bool(void) const {
		return bool(opt);
	}
	DBCacheOptionalBase(void) = default;
	DBCacheOptionalBase(const DBCacheOptionalBase&) = delete;
	DBCacheOptionalBase(DBCacheOptionalBase&&) = default;
	DBCacheOptionalBase& operator=(DBCacheOptionalBase&&) = default;
};
/** Database cache template
 *
 * This template ease implementation of querying the database with a cache.
 */
template <typename KeyT, typename ValueT, typename OptionalT = DBCacheOptionalBase<ValueT>, typename SQLBinder = default_SQLBinder<KeyT>>
struct DBCache {
	using key_type = KeyT;
	using value_type = ValueT;
	using sql_binder_type = SQLBinder;
	using optional_type = OptionalT;
	std::unordered_map<key_type,optional_type> cache_map;
	
	/** Query an artwork
	 * \param key The key to search for
	 * \return The entry, the reference stay valid as long as the DBCache exists.
	 *
	 * \note This function does not perform any SQL transaction. It is meant for
	 *       the JSON parsing stage where the database is unlocked.
	 *       After locking the database, query_db() do this job.
	 */
	optional_type& operator[](const key_type& key) {
		return cache_map[key];
	}
	static const std::string key_to_string(const std::string_view &key) {
		return std::string(key);
	}
	template <typename T>
	static const std::string key_to_string(const T &key) {
		return std::to_string(key);
	}
	/** Fill entries with the database
	 * \param stmt The stmt to run. It must be prepared and accept one bind param,
	 *             that is the key and ValueT objects must be constructible from
	 *             this db and stmt if it just returned SQLITE_ROW and must allow
	 *             future queries.
	 *             In short, I `stmt.bind(1,key)` and `ValueT(db,stmt)`.
s	 */
	void query_db(SQLite3::sqlite3 &db, SQLite3::stmt &stmt, const std::string_view& platform) {
		for (auto &pair: cache_map) {
			stmt.reset();
			SQLBinder().bind(stmt,pair.first,platform);
			switch (stmt.step()) {
				case SQLITE_ROW: {
					pair.second.emplace(db,stmt);
				} break;
				case SQLITE_DONE: {
					// Cache miss, do nothing
				} break;
				default: {
					throw std::runtime_error("Failed to query entry \"" + key_to_string(pair.first) + "\": "+db.errmsg());
				} break;
			}
		}
	}
	
	optional_type &read_json_ref(Arcollect::json::ObjHave have, const char* errmsg, char*& iter, char* const end) {
		return operator[](JSONRefReader<KeyT,DBCache>::read_ref(have,errmsg,iter,end));
	}
};
template <>
struct default_SQLBinder<platform_id> {
	static int bind(SQLite3::stmt &stmt, const platform_id& key) {
		return key.bind(stmt,1);
	}
	static int bind(SQLite3::stmt &stmt, const platform_id& key, const std::string_view& platform) {
		int res = bind(stmt,key);
		if (res == SQLITE_OK)
			res = stmt.bind(2,platform);
		return res;
	}
};

static int bind_timestamp(std::unique_ptr<SQLite3::stmt> &stmt, int pos, sqlite3_int64 timestamp)
{
	return timestamp ? stmt->bind(pos,timestamp) : stmt->bind_null(pos);
}

/** Links parsing template
 */
template <typename vectorT, typename ...Args>
void parse_links(const std::string &array_name, Arcollect::json::ObjHave entry_have, char*& iter, char *const end, vectorT &vector, Args&&... args)
{
	using namespace Arcollect::json;
	if (entry_have != ObjHave::ARRAY)
		throw std::runtime_error("\""+array_name+"\" must be an array.");
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tProcessing \""+array_name+"\"..." << std::endl;
	for (ArrHave have: Arcollect::json::Array(iter,end)) {
		if (have != ArrHave::OBJECT)
			std::runtime_error("\""+array_name+"\" elements must be objects.");
		vector.emplace_back(iter,end,std::forward<Args>(args)...);
	};
}

template <typename T, typename cacheT>
struct basic_cached: public T {
	cacheT &cache;
	template <typename DBCacheT>
	basic_cached(T&& base, DBCacheT& db_cache) : T(std::move(base)), cache(db_cache[this->cache_id()]) {};
};
