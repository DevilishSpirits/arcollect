#ifdef __unix__
#include <unistd.h>
#endif
#include <iostream>
#include <fstream>
#include <vector>
#include <arcollect-db-open.hpp>
#include <arcollect-paths.hpp>
#include "../subprojects/rapidjson/include/rapidjson/document.h"
#include "base64.hpp"

struct new_artwork {
	const char* art_title;
	const char* art_desc;
	const char* art_source;
	sqlite_int64 art_id;
	std::string data;
	// TODO Artwork datas
	new_artwork(rapidjson::Value::ConstValueIterator iter) : 
		art_title(iter->operator[]("title").GetString()),
		art_desc(iter->operator[]("desc").GetString()),
		art_source(iter->operator[]("source").GetString()),
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
	// TODO Artwork datas
	new_account(rapidjson::Value::ConstValueIterator iter) : 
		acc_platid_str(NULL),
		acc_name(iter->operator[]("name").GetString()),
		acc_title(iter->operator[]("title").GetString()),
		acc_url(iter->operator[]("url").GetString()),
		acc_arcoid(-1)
	{
		macaron::Base64::Decode(std::string(iter->operator[]("icon").GetString()),icon_data);
		if (iter->operator[]("id").IsInt64())
			acc_platid_int = iter->operator[]("id").GetInt64();
		else acc_platid_str = iter->operator[]("id").GetString();
	};
};

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
	std::unique_ptr<SQLite3::sqlite3> db = Arcollect::db::open();
	std::string json_string;
	while (std::cin.good()) {
		// Read the JSON
		uint32_t data_len;
		std::cin.read(reinterpret_cast<char*>(&data_len),sizeof(data_len));
		// Quit on empty message
		if (data_len == 0)
			return 0;
		json_string.resize(data_len);
		std::cin.read(json_string.data(),data_len);
		// Parse the JSON
		rapidjson::Document json_dom;
		if (json_dom.ParseInsitu(json_string.data()).HasParseError()) {
			// FIXME Try to give more informations
			std::cerr << "JSON parse error." << std::endl;
			return 1;
		}
		// Get some constants platform
		const bool debug = json_dom.HasMember("debug") && json_dom["debug"].GetBool();
		const std::string platform = json_dom["platform"].GetString();
		if (debug) {
			std::cerr << "Debug mode enabled." << std::endl
			          << "Platform: " << platform << std::endl;
		}
		// Parse the DOM
		std::vector<new_artwork> new_artworks;
		std::vector<new_account> new_accounts;
		
		if (json_dom.HasMember("artworks")) {
			auto &json_arts = json_dom["artworks"];
			if (!json_arts.IsArray()) {
				std::cerr << "\"artworks\" must be an array" << std::endl;
				return 1;
			}
			
			for (rapidjson::Value::ConstValueIterator art_iter = json_arts.Begin(); art_iter != json_arts.End(); ++art_iter) {
				if (!art_iter->IsObject()) {
					std::cerr << "\"artworkss\" elements must be objects" << std::endl;
					return 1;
				}
				new_artworks.emplace_back(art_iter);
			}
		}
		if (json_dom.HasMember("accounts")) {
			auto &json_accs = json_dom["accounts"];
			if (!json_accs.IsArray()) {
				std::cerr << "\"accounts\" must be an array" << std::endl;
				return 1;
			}
			
			for (rapidjson::Value::ConstValueIterator acc_iter = json_accs.Begin(); acc_iter != json_accs.End(); ++acc_iter) {
				if (!acc_iter->IsObject()) {
					std::cerr << "\"accounts\" elements must be objects" << std::endl;
					return 1;
				}
				new_accounts.emplace_back(acc_iter);
			}
		}
		// Debug the transaction
		if (debug) {
			std::cerr << new_artworks.size() << " artwork(s) :" << std::endl;
			for (auto& artwork : new_artworks)
				std::cerr << "\t\"" << artwork.art_title << "\" (" << artwork.art_source << ")\n\t\t" << artwork.art_desc << std::endl << std::endl;
			std::cerr << new_accounts.size() << " account(s) :" << std::endl;
			for (auto& account : new_accounts)
				std::cerr << "\t\"" << account.acc_title << "\" (" << account.acc_name << ") [" << (account.acc_platid_str ? account.acc_platid_str : std::to_string(account.acc_platid_int).c_str()) << "]" << std::endl << std::endl;
		}
		// Perform transaction
		std::unique_ptr<SQLite3::stmt> add_artwork_stmt;
		if (db->prepare("INSERT OR FAIL INTO artworks (art_title,art_platform,art_desc,art_source) VALUES (?,?,?,?) RETURNING art_artid;",add_artwork_stmt)) {
			std::cerr << "Failed to prepare the add_artwork_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& artwork : new_artworks) {
			add_artwork_stmt->bind(1,artwork.art_title);
			add_artwork_stmt->bind(2,platform.c_str());
			add_artwork_stmt->bind(3,artwork.art_desc);
			add_artwork_stmt->bind(4,artwork.art_source);
			add_artwork_stmt->bind(5,artwork.art_title);
			switch (add_artwork_stmt->step()) {
				case SQLITE_ROW: {
					artwork.art_id = add_artwork_stmt->column_int64(0);
					std::ofstream artwork_file(Arcollect::db::artwork_pool_path+std::to_string(artwork.art_id));
					artwork_file << artwork.data;
				} break;
				case SQLITE_DONE: {
				} break;
				default: {
					std::cerr << "Error executing the STMT" << db->errmsg() << std::endl;
				} break;
			}
			add_artwork_stmt->reset();
		}
		
		std::unique_ptr<SQLite3::stmt> add_account_stmt;
		if (db->prepare("INSERT OR IGNORE INTO accounts (acc_platid,acc_platform,acc_name,acc_title,acc_url) VALUES (?,?,?,?,?) RETURNING acc_arcoid;",add_account_stmt)) {
			std::cerr << "Failed to prepare the add_account_stmt " << db->errmsg() << std::endl;
			return 1;
		}
		for (auto& account : new_accounts) {
			if (account.acc_platid_str)
				add_account_stmt->bind(1,account.acc_platid_str);
			else add_account_stmt->bind(1,account.acc_platid_int);
			add_account_stmt->bind(2,platform.c_str());
			add_account_stmt->bind(3,account.acc_name);
			add_account_stmt->bind(4,account.acc_title);
			add_account_stmt->bind(5,account.acc_url);
			switch (add_account_stmt->step()) {
				case SQLITE_ROW: {
					account.acc_arcoid = add_account_stmt->column_int64(0);
					std::ofstream account_file(Arcollect::db::account_avatars_path+std::to_string(account.acc_arcoid));
					account_file << account.icon_data;
				} break;
				case SQLITE_DONE: {
				} break;
				default: {
					std::cerr << "Error executing the STMT" << db->errmsg() << std::endl;
				} break;
			}
			add_account_stmt->reset();
		}
		db->exec("COMMIT;");
		// Return
		std::string result_js = "{}";
		data_len = result_js.size();
		std::cout.write(reinterpret_cast<char*>(&data_len),sizeof(data_len));
		std::cout << result_js;
	}
}
