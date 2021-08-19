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
#include <curl/curl.h>
#include <config.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <optional>
#include <vector>
#include <arcollect-paths.hpp>
#include <arcollect-sqls.hpp>
#include <rapidjson/document.h>
#include "base64.hpp"

extern std::unique_ptr<SQLite3::sqlite3> db;
extern const std::string user_agent;

const std::string user_agent = "Arcollect/" ARCOLLECT_VERSION_STR " curl/" + std::string(curl_version_info(CURLVERSION_NOW)->version);
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
static const char* json_string(rapidjson::Value::ConstValueIterator iter, const char* key, const char* default_value = NULL)
{
	auto& object = *iter;
	if (object.HasMember(key) && object[key].IsString()) {
		return object[key].GetString();
	} else return default_value;
}

static std::optional<std::string> data_saveto(const char* data_string, std::filesystem::path target, const char* referer)
{
	// Check for "https://" schema
	const static char https_prefix[] = {'h','t','t','p','s',':','/','/'}; // "https://" without the '\0'
	if (strncmp(https_prefix,data_string,sizeof(https_prefix)) == 0) {
		// TODO Error handling
		char curl_errorbuffer[CURL_ERROR_SIZE];
		std::optional<std::string> result;
		FILE* file = fopen(target.string().c_str(),"wb");
		auto easyhandle = curl_easy_init(); 
		curl_easy_setopt(easyhandle,CURLOPT_URL,data_string);
		curl_easy_setopt(easyhandle,CURLOPT_WRITEDATA,file);
		curl_easy_setopt(easyhandle,CURLOPT_PROTOCOLS,CURLPROTO_HTTPS);
		curl_easy_setopt(easyhandle,CURLOPT_REFERER,referer);
		curl_easy_setopt(easyhandle,CURLOPT_USERAGENT,user_agent.c_str());
		curl_easy_setopt(easyhandle,CURLOPT_ERRORBUFFER,curl_errorbuffer);
		
		CURLcode curl_res = curl_easy_perform(easyhandle);
		fclose(file);
		
		if (curl_res != CURLE_OK) {
			result = std::string(curl_errorbuffer);
			std::filesystem::remove(target);
		}
		curl_easy_cleanup(easyhandle);
		return result;
	} else {
		// Assume base64 encoding
		// TODO Decode in-place
		std::string binary;
		macaron::Base64::Decode(data_string,binary);
		std::ofstream(target) << binary;
		return std::nullopt;
	}
}

/** Helper struct storing either a sqlite_int64 or a std::string
 */
struct platform_id {
	std::string_view  platid_str;
	sqlite_int64 platid_int;
	
	bool IsInt64(void) const {
		return platid_int >= 0 ;
	}
	operator sqlite_int64(void) const {
		return platid_int;
	};
	operator const std::string_view&(void) const {
		return platid_str;
	};
	
	/** Bind a SQLite stmt value
	 */
	int bind(std::unique_ptr<SQLite3::stmt> &stmt, int col) const {
		if (IsInt64())
			return stmt->bind(col,platid_int);
		else return stmt->bind(col,platid_str.data());
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
	bool operator==(const platform_id& other) const {
		if (IsInt64() && other.IsInt64())
			return platid_int == other.platid_int;
		else if (!IsInt64() && !other.IsInt64())
			return platid_str == other.platid_str;
			// Type mismatch, convert this or other numeric value into std::string
		else if (IsInt64()) // && !other.IsInt64() implied
			return std::to_string(platid_int) == other.platid_str;
		else //if (!IsInt64() && other.IsInt64() implied
			return platid_str == std::to_string(other.platid_int);
	}
};
namespace std {
	template<> struct hash<platform_id> {
		std::size_t operator()(const platform_id& plat) const noexcept {
			if (plat.IsInt64())
				return std::hash<sqlite_int64>{}(static_cast<sqlite_int64>(plat));
			else return std::hash<std::string_view>{}(static_cast<std::string_view>(plat));
		}
	};
}
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
	const char* art_mimetype;
	sqlite_int64 art_postdate;
	sqlite_int64 art_id;
	const char* thumbnail;
	const char* data;
	static constexpr char default_art_mimetype[] = "image/*";
	new_artwork(rapidjson::Value::ConstValueIterator iter) : 
		art_title(json_string(iter,"title")),
		art_desc(json_string(iter,"desc")),
		art_source(json_string(iter,"source")),
		art_rating(json_int64(iter,"rating",0)),
		art_mimetype(json_string(iter,"mimetype",default_art_mimetype)),
		art_postdate(json_int64(iter,"postdate",0)),
		art_id(-1),
		thumbnail(json_string(iter,"thumbnail")),
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

std::unique_ptr<SQLite3::stmt> find_artwork_stmt;
sqlite_int64 find_artwork(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<std::string_view,new_artwork> &new_artworks, const std::string_view &url)
{
	auto iter = new_artworks.find(url);
	if ((iter == new_artworks.end())||(iter->second.art_id < 0)) {
		// TODO Error checkings
		find_artwork_stmt->reset();
		find_artwork_stmt->bind(1,url.data());
		find_artwork_stmt->step();
		return find_artwork_stmt->column_int64(0);
	} else return iter->second.art_id;
}
std::unique_ptr<SQLite3::stmt> find_account_stmt;
sqlite_int64 find_account(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<platform_id,new_account> &new_accounts, const platform_id& acc_platid)
{
	auto iter = new_accounts.find(acc_platid);
	if ((iter == new_accounts.end())||(iter->second.acc_arcoid < 0)) {
		// TODO Error checkings
		find_account_stmt->reset();
		acc_platid.bind(find_account_stmt,1);
		find_account_stmt->step();
		return find_account_stmt->column_int64(0);
	} else return iter->second.acc_arcoid;
}
std::unique_ptr<SQLite3::stmt> find_tag_stmt;
sqlite_int64 find_tag(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<platform_id,new_tag> &new_tags, const platform_id& tag_platid)
{
	auto iter = new_tags.find(tag_platid);
	if ((iter == new_tags.end())||(iter->second.tag_arcoid < 0)) {
		// TODO Error checkings
		find_tag_stmt->reset();
		tag_platid.bind(find_tag_stmt,1);
		find_tag_stmt->step();
		return find_tag_stmt->column_int64(0);
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

extern bool debug;

static std::optional<std::string> do_add(rapidjson::Document &json_dom)
{
	if (debug)
		std::cerr << "JSON parsed. Reading DOM..." << std::endl;
	// Get some constants platform
	const std::string platform = json_dom["platform"].GetString();
	if (debug)
		std::cerr << "Platform: " << platform << std::endl;
	// Parse the DOM
	std::unordered_map<std::string_view,new_artwork> new_artworks = json_parse_objects<decltype(new_artworks)>(json_dom,"artworks",
		[](decltype(new_artworks)& new_artworks, rapidjson::Value::ConstValueIterator art_iter) {
			new_artworks.emplace(std::string_view(art_iter->operator[]("source").GetString()),art_iter);
	});
	
	std::unordered_map<platform_id,new_account> new_accounts = json_parse_objects<decltype(new_accounts)>(json_dom,"accounts",
		[](decltype(new_accounts)& new_accounts, rapidjson::Value::ConstValueIterator acc_iter) {
			new_accounts.emplace(acc_iter->operator[]("id"),acc_iter);
	});
	
	std::unordered_map<platform_id,new_tag> new_tags = json_parse_objects<decltype(new_tags)>(json_dom,"tags",
		[](decltype(new_tags)& new_tags, rapidjson::Value::ConstValueIterator tag_iter) {
			new_tags.emplace(tag_iter->operator[]("id"),tag_iter);
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
			std::cerr << "\t\"" << artwork.second.art_title << "\" (" << artwork.second.art_source << ")\n" << std::endl;
		std::cerr << new_accounts.size() << " account(s) :" << std::endl;
		for (auto& account : new_accounts)
			std::cerr << "\t\"" << account.second.acc_title << "\" (" << account.second.acc_name << ") [" << account.second.acc_platid << "]\n" << std::endl;
		std::cerr << new_art_acc_links.size() << " artwork/account link(s) :" << std::endl;
		/* TODO
		for (auto& art_acc_link : new_art_acc_links)
			std::cerr << "\t\"" << art_acc_link. << "\" (" << account.second.acc_name << ") [" << (account.second.acc_platid_str ? account.second.acc_platid_str : std::to_string(account.second.acc_platid_int).c_str()) << "]" << std::endl << std::endl;
		*/
	}
	// Prepare SELECT transactions
	if (db->prepare("SELECT art_artid FROM artworks WHERE art_source = ?;",find_artwork_stmt))
		return "Failed to prepare the find_artwork_stmt " + std::string(db->errmsg());
	if (db->prepare("SELECT acc_arcoid FROM accounts WHERE acc_platid = ?;",find_account_stmt))
		return "Failed to prepare the find_account_stmt " + std::string(db->errmsg());
	if (db->prepare("SELECT tag_arcoid FROM tags WHERE tag_platid = ?;",find_tag_stmt))
		return "Failed to prepare the find_tag_stmt " + std::string(db->errmsg());
	// Perform transaction
	int begin_code = db->exec("BEGIN IMMEDIATE;");
	switch (begin_code) {
		case SQLITE_OK:
			break; // It's okay!
		case SQLITE_BUSY:
			return "Arcollect database is locked by another process.";
		default:
			return "Failed to begin database transaction: " + std::string(db->errmsg());
	}
	if (debug)
		std::cerr << "Started SQLite transaction" << std::endl;
	// INSERT INTO artworks
	std::unique_ptr<SQLite3::stmt> insert_stmt;
	if (db->prepare(Arcollect::db::sql::adder_insert_artwork.c_str(),insert_stmt)) {
		std::cerr << "Failed to prepare adder_insert_artwork.sql " << db->errmsg() << std::endl;
		std::exit(1);
	}
	for (auto& artwork : new_artworks) {
		insert_stmt->bind(1,artwork.second.art_title);
		insert_stmt->bind(2,platform.c_str());
		insert_stmt->bind(3,artwork.second.art_desc);
		insert_stmt->bind(4,artwork.second.art_source);
		insert_stmt->bind(5,artwork.second.art_rating);
		insert_stmt->bind(6,artwork.second.art_mimetype);
		if (artwork.second.art_postdate)
			insert_stmt->bind(7,artwork.second.art_postdate);
		else insert_stmt->bind_null(7);
		switch (insert_stmt->step()) {
			case SQLITE_ROW: {
				// Save artwork
				artwork.second.art_id = insert_stmt->column_int64(0);
				auto saveto_res = data_saveto(artwork.second.data,Arcollect::path::artwork_pool / std::to_string(artwork.second.art_id),artwork.second.art_source);
				if (saveto_res)
					return saveto_res;
				if (artwork.second.thumbnail) {
					saveto_res = data_saveto(artwork.second.thumbnail,Arcollect::path::artwork_pool / (std::to_string(artwork.second.art_id)+".thumbnail"),artwork.second.art_source);
				if (saveto_res)
					return saveto_res;
				}
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
		std::exit(1);
	}
	if (db->prepare(Arcollect::db::sql::adder_insert_account.c_str(),insert_stmt)) {
		std::cerr << "Failed to prepare adder_insert_account.sql " << db->errmsg() << std::endl;
		std::exit(1);
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
						auto saveto_res = data_saveto(account.second.icon_data,Arcollect::path::account_avatars / std::to_string(account.second.acc_arcoid),account.second.acc_url);
						if (saveto_res)
							return saveto_res;
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
		std::exit(1);
	}
	if (db->prepare(Arcollect::db::sql::adder_insert_tag.c_str(),insert_stmt)) {
		std::cerr << "Failed to prepare adder_insert_tag.sql " << db->errmsg() << std::endl;
		std::exit(1);
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
		std::exit(1);
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
		std::exit(1);
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
				std::cerr << "INSERT INTO art_tag_links (tag_arcoid, art_artid) VALUES (" << find_tag(db,new_tags,art_tag_link.tag_platid) << "," << find_artwork(db,new_artworks,art_tag_link.art_source) << ") failed: " << db->errmsg() << std::endl;
			} break;
		}
		insert_stmt->reset();
	}
	// Return success
	return std::nullopt;
}

static std::string escape_json(const char* input)
{
	std::string output;
	for (;*input;input++)
		switch (*input) {
			case '\\':output += "\\\\";break;
			case '"' :output += "\\\'";break;
			case '\0':output += "\\u0000";break;
			case 0x01:output += "\\u0001";break;
			case 0x02:output += "\\u0002";break;
			case 0x03:output += "\\u0003";break;
			case 0x04:output += "\\u0004";break;
			case 0x05:output += "\\u0005";break;
			case 0x06:output += "\\u0006";break;
			case 0x07:output += "\\u0007";break;
			case '\b':output += "\\b";break;
			case '\t':output += "\\t";break;
			case '\n':output += "\\n";break;
			case 0x0B:output += "\\u000B";break;
			case 0x0C:output += "\\u000C";break;
			case '\r':output += "\\r";break;
			case 0x0E:output += "\\u000E";break;
			case 0x0F:output += "\\u000F";break;
			case 0x11:output += "\\u0011";break;
			case 0x12:output += "\\u0012";break;
			case 0x13:output += "\\u0013";break;
			case 0x14:output += "\\u0014";break;
			case 0x15:output += "\\u0015";break;
			case 0x16:output += "\\u0016";break;
			case 0x17:output += "\\u0017";break;
			case 0x18:output += "\\u0018";break;
			case 0x19:output += "\\u0019";break;
			case 0x1A:output += "\\u001A";break;
			case 0x1B:output += "\\u001B";break;
			case 0x1C:output += "\\u001C";break;
			case 0x1D:output += "\\u001D";break;
			case 0x1E:output += "\\u001E";break;
			case 0x1F:output += "\\u001F";break;
			default  :output += *input;break;
		}
	return output;
}
std::string handle_json_dom(rapidjson::Document &json_dom)
{
	// Get the transaction_id
	const char* transaction_id = NULL;
	if (json_dom.HasMember("transaction_id") && json_dom["transaction_id"].IsString()) {
		transaction_id = json_dom["transaction_id"].GetString();
		if (debug)
			std::cerr << "transaction_id set to \"" << transaction_id << "\"" << std::endl;
	}
	// Perform addition
	std::optional<std::string> reason = do_add(json_dom);
	// Destroy SELECT transactions to unlock the database
	find_artwork_stmt.reset();
	find_account_stmt.reset();
	find_tag_stmt.reset();
	// Commit or rollback
	if (reason)
		db->exec("ROLLBACK;"); // TODO Check errors
	else db->exec("COMMIT;"); // TODO Check errors
	
	std::string result_json = "{\"success\":";
	if (reason)
		result_json += "false";
	else result_json += "true";
	if (transaction_id)
		result_json += ",\"transaction_id\":\"" + escape_json(transaction_id) + "\"";
	if (reason) {
		result_json += ",\"reason\":\"" + escape_json(reason->c_str()) + "\"";
		std::cerr << "Addition failed: " << *reason << std::endl;
	}
	
	return result_json+"}";
}
