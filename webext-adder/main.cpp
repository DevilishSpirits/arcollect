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
#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <arcollect-db-open.hpp>
#include <arcollect-paths.hpp>
#include "../subprojects/rapidjson/include/rapidjson/document.h"
#include "base64.hpp"

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
	if (object.HasMember(key)) {
		return object[key].GetString();
	} else return NULL;
}
struct new_artwork {
	const char* art_title;
	const char* art_desc;
	const char* art_source;
	sqlite_int64 art_rating;
	sqlite_int64 art_id;
	std::string data;
	// TODO Artwork datas
	new_artwork(rapidjson::Value::ConstValueIterator iter) : 
		art_title(json_string(iter,"title")),
		art_desc(json_string(iter,"desc")),
		art_source(json_string(iter,"source")),
		art_rating(json_int64(iter,"rating",0)),
		art_id(-1)
	{
		macaron::Base64::Decode(std::string(iter->operator[]("data").GetString()),data);
	};
};

struct new_account {
	const char*  acc_platid_str;
	sqlite_int64 acc_platid_int;
	const char*  acc_name;
	const char*  acc_title;
	const char*  acc_url;
	
	sqlite_int64 acc_arcoid;
	std::string icon_data;
	new_account(rapidjson::Value::ConstValueIterator iter) : 
		acc_platid_str(NULL),
		acc_name(json_string(iter,"name")),
		acc_title(json_string(iter,"title")),
		acc_url(json_string(iter,"url")),
		acc_arcoid(-1)
	{
		macaron::Base64::Decode(std::string(iter->operator[]("icon").GetString()),icon_data);
		if (iter->operator[]("id").IsInt64())
			acc_platid_int = iter->operator[]("id").GetInt64();
		else acc_platid_str = iter->operator[]("id").GetString();
	};
};

struct new_art_acc_link {
	const char*  acc_platid_str;
	sqlite_int64 acc_platid_int;
	const char*  art_source;
	const char*  artacc_link;
	
	new_art_acc_link(rapidjson::Value::ConstValueIterator iter) : 
		acc_platid_str(NULL),
		art_source(json_string(iter,"artwork")),
		artacc_link(json_string(iter,"link"))
	{
		if (iter->operator[]("account").IsInt64())
			acc_platid_int = iter->operator[]("account").GetInt64();
		else acc_platid_str = iter->operator[]("account").GetString();
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
sqlite_int64 find_account(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<std::string,new_account> &new_accounts, const char* acc_platid_str, sqlite_int64 acc_platid_int)
{
	std::string acc_platid_string = acc_platid_str ? std::string(acc_platid_str) : std::to_string(acc_platid_int);
	auto iter = new_accounts.find(acc_platid_string);
	if ((iter == new_accounts.end())||(iter->second.acc_arcoid < 0)) {
		// TODO Error checkings
		std::unique_ptr<SQLite3::stmt> select_stmt;
		db->prepare("SELECT acc_arcoid FROM accounts WHERE acc_platid = ?;",select_stmt);
		if (acc_platid_str)
			select_stmt->bind(1,acc_platid_str);
		else select_stmt->bind(1,acc_platid_int);
		select_stmt->step();
		return select_stmt->column_int64(0);
	} else return iter->second.acc_arcoid;
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
			"Arcollect web extension adder\n\n"
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
		
		std::vector<new_art_acc_link> new_art_acc_links = json_parse_objects<decltype(new_art_acc_links)>(json_dom,"art_acc_links",
			[](decltype(new_art_acc_links)& new_art_acc_links, rapidjson::Value::ConstValueIterator link_iter) {
				new_art_acc_links.emplace_back(link_iter);
		});
		// Debug the transaction
		if (debug) {
			std::cerr << new_artworks.size() << " artwork(s) :" << std::endl;
			for (auto& artwork : new_artworks)
				std::cerr << "\t\"" << artwork.second.art_title << "\" (" << artwork.second.art_source << ")\n\t\t" << artwork.second.art_desc << std::endl << std::endl;
			std::cerr << new_accounts.size() << " account(s) :" << std::endl;
			for (auto& account : new_accounts)
				std::cerr << "\t\"" << account.second.acc_title << "\" (" << account.second.acc_name << ") [" << (account.second.acc_platid_str ? account.second.acc_platid_str : std::to_string(account.second.acc_platid_int).c_str()) << "]" << std::endl << std::endl;
			std::cerr << new_art_acc_links.size() << " artwork/account link(s) :" << std::endl;
			/* TODO
			for (auto& art_acc_link : new_art_acc_links)
				std::cerr << "\t\"" << art_acc_link. << "\" (" << account.second.acc_name << ") [" << (account.second.acc_platid_str ? account.second.acc_platid_str : std::to_string(account.second.acc_platid_int).c_str()) << "]" << std::endl << std::endl;
			*/
		}
		// Perform transaction
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
					std::ofstream artwork_file(Arcollect::path::artwork_pool / std::to_string(artwork.second.art_id));
					artwork_file << artwork.second.data;
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
			if (account.second.acc_platid_str)
				get_account_stmt->bind(2,account.second.acc_platid_str);
			else get_account_stmt->bind(2,account.second.acc_platid_int);
			switch (get_account_stmt->step()) {
				case SQLITE_ROW: {
					// User already exist, save acc_arcoid
					account.second.acc_arcoid = get_account_stmt->column_int64(0);
				} break;
				case SQLITE_DONE: {
					// User does not exist, create it
					if (account.second.acc_platid_str)
						insert_stmt->bind(1,account.second.acc_platid_str);
					else insert_stmt->bind(1,account.second.acc_platid_int);
					insert_stmt->bind(2,platform.c_str());
					insert_stmt->bind(3,account.second.acc_name);
					insert_stmt->bind(4,account.second.acc_title);
					insert_stmt->bind(5,account.second.acc_url);
					switch (insert_stmt->step()) {
						case SQLITE_ROW: {
							// Save profile icon
							account.second.acc_arcoid = insert_stmt->column_int64(0);
							std::ofstream account_file(Arcollect::path::account_avatars / std::to_string(account.second.acc_arcoid));
							account_file << account.second.icon_data;
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
		
		// INSERT INTO art_acc_links
		if (db->prepare("INSERT OR FAIL INTO art_acc_links (acc_arcoid, art_artid, artacc_link) VALUES (?,?,?);",insert_stmt)) {
			std::cerr << "Failed to prepare the add_art_acc_links_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& art_acc_link : new_art_acc_links) {
			insert_stmt->bind(1,find_account(db,new_accounts,art_acc_link.acc_platid_str,art_acc_link.acc_platid_int));
			insert_stmt->bind(2,find_artwork(db,new_artworks,art_acc_link.art_source));
			insert_stmt->bind(3,art_acc_link.artacc_link);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
				} break;
				case SQLITE_DONE: {
				} break;
				default: {
					std::cerr << "INSERT INTO art_acc_links (acc_arcoid, art_artid, artacc_link) VALUES (" << find_account(db,new_accounts,art_acc_link.acc_platid_str,art_acc_link.acc_platid_int) << "," << find_artwork(db,new_artworks,art_acc_link.art_source) << ",\"" << art_acc_link.artacc_link << "\") failed: " << db->errmsg() << std::endl;
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
