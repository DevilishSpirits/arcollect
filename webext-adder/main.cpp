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
#ifdef __unix__
#include <unistd.h>
#endif
#include <curl/curl.h>
#include <config.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <arcollect-db-open.hpp>
#include <arcollect-paths.hpp>
#include <rapidjson/document.h>
#include "base64.hpp"

static const std::string user_agent = "Arcollect/" ARCOLLECT_VERSION_STR " curl/" + std::string(curl_version_info(CURLVERSION_NOW)->version);
/** RapidJSON helper to add integers
 * \param iter           Iterator to the object
 * \param key            The key to read
 * \param default_value  The value if the key does not exist
 * \return default_value is the key doesn't exist or it's string value
 */
static sqlite_int64 json_int64(rapidjson::Value::ConstValueIterator iter, const char* key, sqlite_int64 default_value)
{
	auto& object = *iter;
	if (object.HasMember(key)) {
		return object[key].GetInt64();
	} else return default_value;
}
/** RapidJSON helper to add strings
 * \param iter Iterator to the object
 * \param key  The key to read
 * \return NULL is the key doesn't exist or it's string value
 */
static const char* json_string(rapidjson::Value::ConstValueIterator iter, const char* key)
{
	auto& object = *iter;
	if (object.HasMember(key) && object[key].IsString()) {
		return object[key].GetString();
	} else return NULL;
}

static void data_saveto(const char* data_string, std::string target, const char* referer)
{
	// Check for "https://" schema (and it take 8 bytes exactly...)
	// TODO FIXME Check data_string length !!!
	if (*reinterpret_cast<const uint64_t*>(data_string) == *reinterpret_cast<const uint64_t*>("https://")) {
		// TODO Error handling
		FILE* file = fopen(target.c_str(),"wb");
		auto easyhandle = curl_easy_init(); 
		curl_easy_setopt(easyhandle,CURLOPT_URL,data_string);
		curl_easy_setopt(easyhandle,CURLOPT_WRITEDATA,file);
		curl_easy_setopt(easyhandle,CURLOPT_PROTOCOLS,CURLPROTO_HTTPS); // FIXME Should I be HTTPS-only ?
		curl_easy_setopt(easyhandle,CURLOPT_REFERER,referer);
		curl_easy_setopt(easyhandle,CURLOPT_USERAGENT,user_agent.c_str());
		
		curl_easy_perform(easyhandle); // TODO Error checkings
		curl_easy_cleanup(easyhandle);
		fclose(file);
	} else {
		// Assume base64 encoding
		// TODO Decode in-place
		std::string binary;
		macaron::Base64::Decode(data_string,binary);
		std::ofstream(target) << binary;
	}
}

/** Helper struct storing either a sqlite_int64 or a std::string
 */
struct platform_id {
	std::string  platid_str;
	sqlite_int64 platid_int;
	
	bool IsInt64(void) const {
		return platid_int >= 0 ;
	}
	operator sqlite_int64(void) const {
		return platid_int;
	};
	operator const std::string(void) const {
		return platid_str;
	};
	operator const char*(void) const {
		return platid_str.c_str();
	};
	
	/** Bind a SQLite stmt value
	 */
	int bind(std::unique_ptr<SQLite3::stmt> &stmt, int col) const {
		if (IsInt64())
			return stmt->bind(col,platid_int);
		else return stmt->bind(col,platid_str.c_str());
	}
	
	platform_id(const rapidjson::GenericValue<rapidjson::UTF8<>> &value) {
		if (value.IsInt64()) {
			platid_int = value.GetInt64();
			platid_str = std::to_string(platid_int);
		} else {
			platid_int = -1;
			platid_str = value.GetString();
		}
	}
};
std::ostream &operator<<(std::ostream &left, const platform_id &right) {
	if (right.IsInt64())
		left << right.platid_int;
	else left << right.platid_str;
	return left;
}

struct new_artwork {
	const char* art_title;
	const char* art_desc;
	const char* art_source;
	sqlite_int64 art_rating;
	sqlite_int64 art_id;
	const char* data;
	// TODO Artwork datas
	new_artwork(rapidjson::Value::ConstValueIterator iter) : 
		art_title(json_string(iter,"title")),
		art_desc(json_string(iter,"desc")),
		art_source(json_string(iter,"source")),
		art_rating(json_int64(iter,"rating",0)),
		art_id(-1),
		data(iter->operator[]("data").GetString())
	{
	};
};

struct new_account {
	platform_id  acc_platid;
	const char*  acc_name;
	const char*  acc_title;
	const char*  acc_url;
	
	sqlite_int64 acc_arcoid;
	const char* icon_data;
	new_account(rapidjson::Value::ConstValueIterator iter) : 
		acc_platid(iter->operator[]("id")),
		acc_name(json_string(iter,"name")),
		acc_title(json_string(iter,"title")),
		acc_url(json_string(iter,"url")),
		acc_arcoid(-1),
		icon_data(iter->operator[]("icon").GetString())
	{
	};
};

struct new_tag {
	platform_id  tag_platid;
	const char*  tag_title;
	const char*  tag_kind;
	
	sqlite_int64 tag_arcoid;
	
	new_tag(rapidjson::Value::ConstValueIterator iter) : 
		tag_platid(iter->operator[]("id")),
		tag_title(json_string(iter,"title")),
		tag_kind(json_string(iter,"kind")),
		tag_arcoid(-1)
	{
	};
};

struct new_art_acc_link {
	platform_id  acc_platid;
	const char*  art_source;
	const char*  artacc_link;
	
	new_art_acc_link(rapidjson::Value::ConstValueIterator iter) : 
		acc_platid(iter->operator[]("account")),
		art_source(json_string(iter,"artwork")),
		artacc_link(json_string(iter,"link"))
	{
	};
};

struct new_art_tag_link {
	platform_id  tag_platid;
	const char*  art_source;
	
	new_art_tag_link(rapidjson::Value::ConstValueIterator iter) : 
		tag_platid(iter->operator[]("tag")),
		art_source(json_string(iter,"artwork"))
	{
	};
};

sqlite_int64 find_artwork(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<std::string,new_artwork> &new_artworks, const std::string &url)
{
	auto iter = new_artworks.find(url);
	if ((iter == new_artworks.end())||(iter->second.art_id < 0)) {
		// TODO Error checkings
		std::unique_ptr<SQLite3::stmt> select_stmt;
		db->prepare("SELECT art_artid FROM artworks WHERE art_source = ?;",select_stmt);
		select_stmt->bind(1,url.c_str());
		select_stmt->step();
		return select_stmt->column_int64(0);
	} else return iter->second.art_id;
}
sqlite_int64 find_account(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<std::string,new_account> &new_accounts, const platform_id& acc_platid)
{
	auto iter = new_accounts.find(acc_platid);
	if ((iter == new_accounts.end())||(iter->second.acc_arcoid < 0)) {
		// TODO Error checkings
		std::unique_ptr<SQLite3::stmt> select_stmt;
		db->prepare("SELECT acc_arcoid FROM accounts WHERE acc_platid = ?;",select_stmt);
		acc_platid.bind(select_stmt,1);
		select_stmt->step();
		return select_stmt->column_int64(0);
	} else return iter->second.acc_arcoid;
}
sqlite_int64 find_tag(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<std::string,new_tag> &new_tags, const platform_id& tag_platid)
{
	auto iter = new_tags.find(tag_platid);
	if ((iter == new_tags.end())||(iter->second.tag_arcoid < 0)) {
		// TODO Error checkings
		std::unique_ptr<SQLite3::stmt> select_stmt;
		db->prepare("SELECT tag_arcoid FROM tags WHERE tag_platid = ?;",select_stmt);
		tag_platid.bind(select_stmt,1);
		select_stmt->step();
		return select_stmt->column_int64(0);
	} else return iter->second.tag_arcoid;
}


template <typename map_type>
map_type json_parse_objects(rapidjson::Document &json_dom, const char* json_key, std::function<void(map_type&,rapidjson::Value::ConstValueIterator)> do_emplace)
{
	map_type new_objects;
	if (json_dom.HasMember(json_key)) {
		auto &json_array = json_dom[json_key];
		if (!json_array.IsArray()) {
			std::cerr << "\"" << json_key << "\" must be an array" << std::endl;
			std::exit(1);
		}
		
		for (rapidjson::Value::ConstValueIterator iter = json_array.Begin(); iter != json_array.End(); ++iter) {
			if (!iter->IsObject()) {
				std::cerr << "\"" << json_key << "\" elements must be objects" << std::endl;
				std::exit(1);
			}
			do_emplace(new_objects,iter);
		}
	}
	return new_objects;
}

int main(void)
{
	#ifdef __unix__
	if (isatty(0) == 1) {
		std::cerr <<
			"Arcollect web extension adder (" << user_agent << ")\n\n"
			"This program is used by web extensions to add new artworks.\n"
			"It works with JSON and is not intended to be used by biological entities.\n"
		<< std::endl;
	}
	// TODO Handle SIGTERM
	#endif
	// Check for debug mode
	const char* arcollect_debug_env = std::getenv("ARCOLLECT_WEBEXT_ADDER_DEBUG");
	bool debug = false;
	if (arcollect_debug_env) {
		// TODO Set a real interface
		debug = true;
		std::cerr << "Arcollect web extension adder debugging on" << std::endl;
	}
	// Main-loop
	std::unique_ptr<SQLite3::sqlite3> db = Arcollect::db::open();
	std::string json_string;
	while (std::cin.good()) {
		// Read the JSON
		uint32_t data_len;
		std::cin.read(reinterpret_cast<char*>(&data_len),sizeof(data_len));
		if (debug)
			std::cerr << "Got a JSON of " << data_len << " chars. Reading..." << std::endl;
		// Quit on empty message
		if (data_len == 0)
			return 0;
		json_string.resize(data_len);
		std::cin.read(json_string.data(),data_len);
		if (debug)
			std::cerr << "JSON is loaded. Parsing..." << std::endl;
		// Parse the JSON
		rapidjson::Document json_dom;
		if (json_dom.ParseInsitu(json_string.data()).HasParseError()) {
			// FIXME Try to give more informations
			std::cerr << "JSON parse error." << std::endl;
			return 1;
		}
		// Get some constants platform
		const std::string platform = json_dom["platform"].GetString();
		if (debug) {
			std::cerr << "JSON parsed. Reading DOM..." << std::endl
			          << "Platform: " << platform << std::endl;
		}
		// Parse the DOM
		std::unordered_map<std::string,new_artwork> new_artworks = json_parse_objects<decltype(new_artworks)>(json_dom,"artworks",
			[](decltype(new_artworks)& new_artworks, rapidjson::Value::ConstValueIterator art_iter) {
				new_artworks.emplace(std::string(art_iter->operator[]("source").GetString()),art_iter);
		});
		
		std::unordered_map<std::string,new_account> new_accounts = json_parse_objects<decltype(new_accounts)>(json_dom,"accounts",
			[](decltype(new_accounts)& new_accounts, rapidjson::Value::ConstValueIterator acc_iter) {
				auto &id_value = acc_iter->operator[]("id");
				std::string map_key = id_value.IsInt64() ? std::to_string(id_value.GetInt64()) : id_value.GetString();
				new_accounts.emplace(map_key,acc_iter);
		});
		
		std::unordered_map<std::string,new_tag> new_tags = json_parse_objects<decltype(new_tags)>(json_dom,"tags",
			[](decltype(new_tags)& new_tags, rapidjson::Value::ConstValueIterator tag_iter) {
				auto &id_value = tag_iter->operator[]("id");
				std::string map_key = id_value.IsInt64() ? std::to_string(id_value.GetInt64()) : id_value.GetString();
				new_tags.emplace(map_key,tag_iter);
		});
		
		std::vector<new_art_acc_link> new_art_acc_links = json_parse_objects<decltype(new_art_acc_links)>(json_dom,"art_acc_links",
			[](decltype(new_art_acc_links)& new_art_acc_links, rapidjson::Value::ConstValueIterator link_iter) {
				new_art_acc_links.emplace_back(link_iter);
		});
		
		std::vector<new_art_tag_link> new_art_tag_links = json_parse_objects<decltype(new_art_tag_links)>(json_dom,"art_tag_links",
			[](decltype(new_art_tag_links)& new_art_tag_links, rapidjson::Value::ConstValueIterator link_iter) {
				new_art_tag_links.emplace_back(link_iter);
		});
		// Debug the transaction
		if (debug) {
			std::cerr << new_artworks.size() << " artwork(s) :" << std::endl;
			for (auto& artwork : new_artworks)
				std::cerr << "\t\"" << artwork.second.art_title << "\" (" << artwork.second.art_source << ")\n\t\t" << artwork.second.art_desc << std::endl << std::endl;
			std::cerr << new_accounts.size() << " account(s) :" << std::endl;
			for (auto& account : new_accounts)
				std::cerr << "\t\"" << account.second.acc_title << "\" (" << account.second.acc_name << ") [" << account.second.acc_platid << "]" << std::endl << std::endl;
			std::cerr << new_art_acc_links.size() << " artwork/account link(s) :" << std::endl;
			/* TODO
			for (auto& art_acc_link : new_art_acc_links)
				std::cerr << "\t\"" << art_acc_link. << "\" (" << account.second.acc_name << ") [" << (account.second.acc_platid_str ? account.second.acc_platid_str : std::to_string(account.second.acc_platid_int).c_str()) << "]" << std::endl << std::endl;
			*/
		}
		// Perform transaction
		db->exec("BEGIN IMMEDIATE;");
		
		// INSERT INTO artworks
		std::unique_ptr<SQLite3::stmt> insert_stmt;
		if (db->prepare("INSERT OR FAIL INTO artworks (art_title,art_platform,art_desc,art_source,art_rating) VALUES (?,?,?,?,?) RETURNING art_artid;",insert_stmt)) {
			std::cerr << "Failed to prepare the add_artwork_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& artwork : new_artworks) {
			insert_stmt->bind(1,artwork.second.art_title);
			insert_stmt->bind(2,platform.c_str());
			insert_stmt->bind(3,artwork.second.art_desc);
			insert_stmt->bind(4,artwork.second.art_source);
			insert_stmt->bind(5,artwork.second.art_rating);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
					// Save artwork
					artwork.second.art_id = insert_stmt->column_int64(0);
					data_saveto(artwork.second.data,Arcollect::path::artwork_pool / std::to_string(artwork.second.art_id),artwork.second.art_source);
				} break;
				case SQLITE_DONE: {
				} break;
				default: {
					std::cerr << "Error executing the STMT" << db->errmsg() << std::endl;
				} break;
			}
			insert_stmt->reset();
		}
		
		// INSERT INTO accounts
		std::unique_ptr<SQLite3::stmt> get_account_stmt;
		if (db->prepare("SELECT acc_arcoid FROM accounts WHERE acc_platform = ? AND acc_platid = ?;",get_account_stmt)) {
			std::cerr << "Failed to prepare the get_account_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		if (db->prepare("INSERT OR FAIL INTO accounts (acc_platid,acc_platform,acc_name,acc_title,acc_url) VALUES (?,?,?,?,?) RETURNING acc_arcoid;",insert_stmt)) {
			std::cerr << "Failed to prepare the add_account_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& account : new_accounts) {
			get_account_stmt->bind(1,platform.c_str());
			account.second.acc_platid.bind(get_account_stmt,2);
			switch (get_account_stmt->step()) {
				case SQLITE_ROW: {
					// User already exist, save acc_arcoid
					account.second.acc_arcoid = get_account_stmt->column_int64(0);
				} break;
				case SQLITE_DONE: {
					// User does not exist, create it
					account.second.acc_platid.bind(insert_stmt,1);
					insert_stmt->bind(2,platform.c_str());
					insert_stmt->bind(3,account.second.acc_name);
					insert_stmt->bind(4,account.second.acc_title);
					insert_stmt->bind(5,account.second.acc_url);
					switch (insert_stmt->step()) {
						case SQLITE_ROW: {
							// Save profile icon
							account.second.acc_arcoid = insert_stmt->column_int64(0);
							std::ofstream account_file(Arcollect::path::account_avatars / std::to_string(account.second.acc_arcoid));
							data_saveto(account.second.icon_data,Arcollect::path::account_avatars / std::to_string(account.second.acc_arcoid),account.second.acc_url);
						} break;
						case SQLITE_DONE: {
						} break;
						default: {
							std::cerr << "Error executing the STMT" << db->errmsg() << std::endl;
						} break;
					}
					insert_stmt->reset();
				} break;
				default: {
					std::cerr << "Error executing the STMT" << db->errmsg() << std::endl;
				} break;
			}
			get_account_stmt->reset();
		}
		
		// INSERT INTO tags
		std::unique_ptr<SQLite3::stmt> get_tag_stmt;
		if (db->prepare("SELECT tag_arcoid FROM tags WHERE tag_platform = ? AND tag_platid = ?;",get_tag_stmt)) {
			std::cerr << "Failed to prepare the get_tag_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		if (db->prepare("INSERT OR FAIL INTO tags (tag_platid,tag_platform,tag_title,tag_kind) VALUES (?,?,?,?) RETURNING tag_arcoid;",insert_stmt)) {
			std::cerr << "Failed to prepare the add_tag_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& tag : new_tags) {
			get_tag_stmt->bind(1,platform.c_str());
			tag.second.tag_platid.bind(get_tag_stmt,2);
			switch (get_tag_stmt->step()) {
				case SQLITE_ROW: {
					// User already exist, save tag_arcoid
					tag.second.tag_arcoid = get_tag_stmt->column_int64(0);
				} break;
				case SQLITE_DONE: {
					// User does not exist, create it
					tag.second.tag_platid.bind(insert_stmt,1);
					insert_stmt->bind(2,platform.c_str());
					insert_stmt->bind(3,tag.second.tag_title);
					insert_stmt->bind(4,tag.second.tag_kind);
					switch (insert_stmt->step()) {
						case SQLITE_ROW: {
						} break;
						case SQLITE_DONE: {
						} break;
						default: {
							std::cerr << "Error executing the STMT" << db->errmsg() << std::endl;
						} break;
					}
					insert_stmt->reset();
				} break;
				default: {
					std::cerr << "Error executing the STMT" << db->errmsg() << std::endl;
				} break;
			}
			get_tag_stmt->reset();
		}
		
		// INSERT INTO art_acc_links
		if (db->prepare("INSERT OR FAIL INTO art_acc_links (acc_arcoid, art_artid, artacc_link) VALUES (?,?,?);",insert_stmt)) {
			std::cerr << "Failed to prepare the add_art_acc_links_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& art_acc_link : new_art_acc_links) {
			insert_stmt->bind(1,find_account(db,new_accounts,art_acc_link.acc_platid));
			insert_stmt->bind(2,find_artwork(db,new_artworks,art_acc_link.art_source));
			insert_stmt->bind(3,art_acc_link.artacc_link);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
				} break;
				case SQLITE_DONE: {
				} break;
				default: {
					std::cerr << "INSERT INTO art_acc_links (acc_arcoid, art_artid, artacc_link) VALUES (" << find_account(db,new_accounts,art_acc_link.acc_platid) << "," << find_artwork(db,new_artworks,art_acc_link.art_source) << ",\"" << art_acc_link.artacc_link << "\") failed: " << db->errmsg() << std::endl;
				} break;
			}
			insert_stmt->reset();
		}
		
		// INSERT INTO art_tag_links
		if (db->prepare("INSERT OR FAIL INTO art_tag_links (tag_arcoid, art_artid) VALUES (?,?);",insert_stmt)) {
			std::cerr << "Failed to prepare the add_art_tag_links_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& art_tag_link : new_art_tag_links) {
			insert_stmt->bind(1,find_tag(db,new_tags,art_tag_link.tag_platid));
			insert_stmt->bind(2,find_artwork(db,new_artworks,art_tag_link.art_source));
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
				} break;
				case SQLITE_DONE: {
				} break;
				default: {
					std::cerr << "INSERT INTO art_tag_links (tag_arcoid, art_artid) VALUES (" << find_tag(db,new_tags,art_tag_link.tag_platid) << "," << find_artwork(db,new_artworks,art_tag_link.art_source) << "\") failed: " << db->errmsg() << std::endl;
				} break;
			}
			insert_stmt->reset();
		}
		
		db->exec("COMMIT;");
		// Return
		std::string result_js = "{}";
		data_len = result_js.size();
		std::cout.write(reinterpret_cast<char*>(&data_len),sizeof(data_len));
		std::cout << result_js;
	}
}
