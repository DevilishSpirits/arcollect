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
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <optional>
#include <vector>
#include <arcollect-debug.hpp>
#include <arcollect-paths.hpp>
#include <arcollect-sqls.hpp>
#include "base64.hpp"
#include "wtf_json_parser-string_view.hpp"
#include "json_escaper.hpp"
using namespace std::literals::string_view_literals;

extern std::unique_ptr<SQLite3::sqlite3> db;
extern const std::string user_agent;

const std::string user_agent = "Arcollect/" ARCOLLECT_VERSION_STR " curl/" + std::string(curl_version_info(CURLVERSION_NOW)->version);

/** Process base64 or https:// "data" fields
 * \param data_string The data string (base64 or https:// link)
 * \param target      Destination file
 * \param referer     Referer to use.
 * \warning `data_string` and `referer` must be NUL-terminated!
 *          The code ensure that with json_read_string_nul_terminate().
 */
static std::optional<std::string> data_saveto(const std::string_view& data_string, const std::filesystem::path& target, const std::string_view &referer)
{
	constexpr static char https_prefix[] = {'h','t','t','p','s',':','/','/'}; // "https://" without the '\0'
	static_assert(sizeof(https_prefix) == sizeof(int64_t));
	// Check for "https://" schema
	// Note: https_prefix is 64bits, I compare the string by casting to an int64_t
	if ((data_string.size() > sizeof(int64_t))&&(*reinterpret_cast<const int64_t*>(data_string.data()) == *reinterpret_cast<const int64_t*>(https_prefix))) {
		// TODO Error handling
		char curl_errorbuffer[CURL_ERROR_SIZE];
		std::optional<std::string> result;
		FILE* file = fopen(target.string().c_str(),"wb");
		auto easyhandle = curl_easy_init(); 
		curl_easy_setopt(easyhandle,CURLOPT_URL,data_string.data());
		curl_easy_setopt(easyhandle,CURLOPT_WRITEDATA,file);
		curl_easy_setopt(easyhandle,CURLOPT_PROTOCOLS,CURLPROTO_HTTPS);
		curl_easy_setopt(easyhandle,CURLOPT_REFERER,referer.data());
		curl_easy_setopt(easyhandle,CURLOPT_USERAGENT,user_agent.c_str());
		curl_easy_setopt(easyhandle,CURLOPT_ERRORBUFFER,curl_errorbuffer);
		
		CURLcode curl_res = curl_easy_perform(easyhandle);
		fclose(file);
		
		if (curl_res != CURLE_OK) {
			if (curl_errorbuffer[0])
				result = std::string(curl_errorbuffer);
			else result = std::string(curl_easy_strerror(curl_res));
			std::filesystem::remove(target);
		}
		curl_easy_cleanup(easyhandle);
		return result;
	} else {
		// Assume base64 encoding
		// TODO Decode in-place
		std::string binary;
		macaron::Base64::Decode(data_string.data(),binary);
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
		else return stmt->bind(col,platid_str);
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
	bool empty(void) const {
		return platid_str.empty();
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

struct JSONParsingError: public std::runtime_error {
	static std::string make_near(char* iter, char* end) {
		if (std::distance(iter,end) > 8)
			end = iter + 8;
		return std::string("Near ")+std::string(iter,std::distance(iter,end));
	}
	JSONParsingError(const std::string& message, char* iter, char* end) :
		std::runtime_error(make_near(iter,end)+": "+message) {}
};

static void json_read_string(Arcollect::json::ObjHave have, std::string_view &out, const std::string& debug_string, char*& iter, char* const end)
{
	using namespace Arcollect::json;
	if (have == ObjHave::NULL_LITTERALLY)
		return; /* Cope fine with NULL */
	if (have != ObjHave::STRING)
		throw JSONParsingError(debug_string+" must be a string.",iter,end);
	if (!read_string(out,iter,end))
		throw JSONParsingError("Error while reading "+debug_string+" string, syntax error.",iter,end);
}
static void json_read_string_nul_terminate(Arcollect::json::ObjHave have, std::string_view &out, const std::string& debug_string, char*& iter, char* const end)
{
	json_read_string(have,out,debug_string,iter,end);
	*const_cast<char*>(out.data() + out.size()) = '\0';
}
static sqlite_int64 json_read_int(Arcollect::json::ObjHave have, const std::string& debug_string, char*& iter, char* const end)
{
	using namespace Arcollect::json;
	sqlite_int64 out;
	if ((have != ObjHave::NUMBER)|| !is_integral_number(iter,end))
		throw JSONParsingError(debug_string+" must be an integral number.",iter,end);
	if (!read_number(out,iter,end))
		throw JSONParsingError("Error while reading "+debug_string+" integral number, syntax error.",iter,end);
	return out;
}

static void read_platform_id(Arcollect::json::ObjHave have, platform_id &out, const std::string& debug_string, char*& iter, char* const end)
{
	using namespace Arcollect::json;
	switch (have) {
		case ObjHave::NUMBER: {
			if (!read_number<decltype(out.platid_int)>(out.platid_int,iter,end))
				throw JSONParsingError("Error while reading "+debug_string+" id as number.",iter,end);
			out.platid_str = std::to_string(out.platid_int);
		} break;
		case ObjHave::STRING: {
			out.platid_int = -1;
			if (!read_string(out.platid_str,iter,end))
				throw JSONParsingError("Error while reading "+debug_string+" id as string.",iter,end);
		} break;
		default:
			throw JSONParsingError(debug_string+" id must be an integral or a string.",iter,end);
	}
	
}

template <typename T>
struct ForEachObjectSwitch {
	using map_type = std::unordered_map<std::string_view,T>;
	const map_type map;
	struct Iteration {
		char*& iter;
		char* const end_iter;
		const map_type &map;
		struct iterator {
			char*& iter;
			char* const end;
			const map_type &map;
			struct ret_type {
				T key;
				Arcollect::json::ObjHave have;
			} ret;
			iterator& operator++(void) {
				using namespace Arcollect::json;
				std::string_view key_name;
				typename map_type::const_iterator map_iter;
				// Loop until we find a key we knows
				while (1) {
					// Check what we have
					ret.have = Arcollect::json::read_object_keyval(key_name,iter,end);
					// On end or error, return
					if (ret.have == ObjHave::WTF)
						throw JSONParsingError("syntax error at object \""+std::string(key_name)+"\" value.",iter,end);
					if (ret.have == ObjHave::OBJECT_CLOSE)
						return *this;
					// Try to find which key is used
					map_iter = map.find(key_name);
					if (map_iter != map.end()) {
						ret.key = map_iter->second;
						return *this;
					} else if (!skip_value(static_cast<Have>(ret.have),iter,end))
						throw JSONParsingError("failed to skip \""+std::string(key_name)+"\", JSON syntax error.",iter,end);
				}
			}
			constexpr bool operator!=(const iterator&) const {
				return ret.have != Arcollect::json::ObjHave::OBJECT_CLOSE;
			}
			constexpr const ret_type& operator*(void) const {
				return ret;
			}
		};
		iterator begin() {
			return ++iterator{iter,end_iter,map};
		}
		iterator end() {
			return iterator{iter,end_iter,map};
		}
	};
	ForEachObjectSwitch(std::initializer_list<typename map_type::value_type> init) : map(init) {}
	Iteration operator()(char*& iter, char* const end) const {
		return Iteration{iter,end,map};
	}
};

struct new_artwork {
	std::string_view art_title;
	std::string_view art_desc;
	std::string_view art_source;
	sqlite_int64     art_rating   = 0;
	std::string_view art_mimetype = "image/*"sv;
	sqlite_int64     art_postdate = 0;
	sqlite_int64     art_id = -1;
	std::string_view thumbnail;
	std::string_view data;
	
	new_artwork(char*& iter, char* const end) {
		enum class Artworks {
			title,
			desc,
			source,
			rating,
			mimetype,
			postdate,
			thumbnail,
			data,
		};
		static const ForEachObjectSwitch<Artworks> artworks_switch{
			{"title"    ,Artworks::title},
			{"desc"     ,Artworks::desc},
			{"source"   ,Artworks::source},
			{"rating"   ,Artworks::rating},
			{"mimetype" ,Artworks::mimetype},
			{"postdate" ,Artworks::postdate},
			{"thumbnail",Artworks::thumbnail},
			{"data"     ,Artworks::data},
		};
		for (auto entry: artworks_switch(iter,end))
			switch (entry.key) {
				case Artworks::title: {
					json_read_string(entry.have,art_title,"\"artworks\":[{\"title\"",iter,end);
				} break;
				case Artworks::desc: {
					json_read_string(entry.have,art_desc,"\"artworks\":[{\"desc\"",iter,end);
				} break;
				case Artworks::source: {
					// NUL-terminate because it is the "referer" param in data_saveto()
					json_read_string_nul_terminate(entry.have,art_source,"\"artworks\":[{\"source\"",iter,end);
				} break;
				case Artworks::rating: {
					art_rating = json_read_int(entry.have,"\"artworks\":[{\"rating\"",iter,end);
				} break;
				case Artworks::mimetype: {
					json_read_string(entry.have,art_mimetype,"\"artworks\":[{\"mimetype\"",iter,end);
				} break;
				case Artworks::postdate: {
					art_postdate = json_read_int(entry.have,"\"artworks\":[{\"postdate\"",iter,end);
				} break;
				case Artworks::thumbnail: {
					// NUL-terminate because it is the "data_string" param in data_saveto()
					json_read_string_nul_terminate(entry.have,thumbnail,"\"artworks\":[{\"thumbnail\"",iter,end);
				} break;
				case Artworks::data: {
					// NUL-terminate because it is the "data_string" param in data_saveto()
					json_read_string_nul_terminate(entry.have,data,"\"artworks\":[{\"data\"",iter,end);
				} break;
		}
	}
};

struct new_account {
	platform_id      acc_platid;
	std::string_view acc_name;
	std::string_view acc_title;
	std::string_view acc_url;
	
	sqlite_int64 acc_arcoid = -1;
	std::string_view icon_data;
	
	new_account(char*& iter, char* const end) {
		enum class Accounts {
			id,
			name,
			title,
			url,
			icon,
		};
		static const ForEachObjectSwitch<Accounts> accounts_switch{
			{"id"   ,Accounts::id},
			{"name" ,Accounts::name},
			{"title",Accounts::title},
			{"url"  ,Accounts::url},
			{"icon" ,Accounts::icon},
		};
		for (auto entry: accounts_switch(iter,end))
			switch (entry.key) {
				case Accounts::id: {
					read_platform_id(entry.have,acc_platid,"\"accounts\":[{\"id\"",iter,end);
				} break;
				case Accounts::name: {
					json_read_string(entry.have,acc_name,"\"accounts\":[{\"name\"",iter,end);
				} break;
				case Accounts::title: {
					json_read_string(entry.have,acc_title,"\"accounts\":[{\"title\"",iter,end);
				} break;
				case Accounts::url: {
					json_read_string_nul_terminate(entry.have,acc_url,"\"accounts\":[{\"url\"",iter,end);
				} break;
				case Accounts::icon: {
					json_read_string_nul_terminate(entry.have,icon_data,"\"accounts\":[{\"icon\"",iter,end);
				} break;
			}
	}
};

struct new_tag {
	platform_id      tag_platid;
	std::string_view tag_title;
	std::string_view tag_kind;
	sqlite_int64 tag_arcoid = -1;
	
	new_tag(char*& iter, char* const end) {
		enum class Tags {
			id,
			title,
			kind,
		};
		static const ForEachObjectSwitch<Tags> tags_switch{
			{"id"   ,Tags::id},
			{"title",Tags::title},
			{"kind" ,Tags::kind},
		};
		for (auto entry: tags_switch(iter,end))
			switch (entry.key) {
				case Tags::id: {
					read_platform_id(entry.have,tag_platid,"\"tags\":[{\"id\"",iter,end);
				} break;
				case Tags::title: {
					json_read_string(entry.have,tag_title,"\"tags\":[{\"title\"",iter,end);
				} break;
				case Tags::kind: {
					json_read_string(entry.have,tag_kind,"\"tags\":[{\"kind\"",iter,end);
				} break;
			}
	}
};

struct new_art_acc_link {
	platform_id      acc_platid;
	std::string_view art_source;
	std::string_view artacc_link;
	
	new_art_acc_link(char*& iter, char* const end) {
		enum class ArtAccLinks {
			artwork,
			account,
			link,
		};
		static const ForEachObjectSwitch<ArtAccLinks> art_acc_links_switch{
			{"artwork",ArtAccLinks::artwork},
			{"account",ArtAccLinks::account},
			{"link"   ,ArtAccLinks::link},
		};
		for (auto entry: art_acc_links_switch(iter,end))
			switch (entry.key) {
				case ArtAccLinks::artwork: {
					json_read_string(entry.have,art_source,"\"art_acc_links\":[{\"artwork\"",iter,end);
				} break;
				case ArtAccLinks::account: {
					read_platform_id(entry.have,acc_platid,"\"art_acc_links\":[{\"account\"",iter,end);
				} break;
				case ArtAccLinks::link: {
					json_read_string(entry.have,artacc_link,"\"art_acc_links\":[{\"link\"",iter,end);
				} break;
			}
	}
};

struct new_art_tag_link {
	platform_id      tag_platid;
	std::string_view art_source;
	
	new_art_tag_link(char*& iter, char* const end) {
		enum class ArtTagLinks {
			artwork,
			tag,
		};
		static const ForEachObjectSwitch<ArtTagLinks> art_tag_links_switch{
			{"artwork",ArtTagLinks::artwork},
			{"tag"    ,ArtTagLinks::tag},
		};
		for (auto entry: art_tag_links_switch(iter,end))
			switch (entry.key) {
				case ArtTagLinks::artwork: {
					json_read_string(entry.have,art_source,"\"art_tag_links\":[{\"artwork\"",iter,end);
				} break;
				case ArtTagLinks::tag: {
					read_platform_id(entry.have,tag_platid,"\"art_tag_links\":[{\"tag\"",iter,end);
				} break;
			}
	}
};

std::unique_ptr<SQLite3::stmt> find_artwork_stmt;
sqlite_int64 find_artwork(std::unique_ptr<SQLite3::sqlite3> &db, std::unordered_map<std::string_view,new_artwork> &new_artworks, const std::string_view &url)
{
	auto iter = new_artworks.find(url);
	if ((iter == new_artworks.end())||(iter->second.art_id < 0)) {
		// TODO Error checkings
		find_artwork_stmt->reset();
		find_artwork_stmt->bind(1,url);
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

static std::optional<std::string> do_add(char* iter, char* const end, std::string_view &transaction_id)
{
	using namespace Arcollect::json;
	std::string_view platform;
	std::unordered_map<std::string_view,new_artwork> new_artworks;
	std::unordered_map<platform_id,new_account> new_accounts;
	std::unordered_map<platform_id,new_tag> new_tags;
	std::vector<new_art_acc_link> new_art_acc_links;
	std::vector<new_art_tag_link> new_art_tag_links;
	if (Arcollect::debug.webext_adder)
		std::cerr << "Parsing JSON..." << std::endl;
	// Read root
	if (what_i_have(iter,end) != Have::OBJECT)
		return "Invalid JSON, expected a root object.";
	
	// Parse JSONs
	enum class Root {
		transaction_id,
		platform,
		artworks,
		accounts,
		tags,
		art_acc_links,
		art_tag_links,
	};
	static const ForEachObjectSwitch<Root> root_switch{
		{"transaction_id",Root::transaction_id},
		{"platform"      ,Root::platform},
		{"artworks"      ,Root::artworks},
		{"accounts"      ,Root::accounts},
		{"tags"          ,Root::tags},
		{"art_acc_links" ,Root::art_acc_links},
		{"art_tag_links" ,Root::art_tag_links},
	};
	for (auto entry: root_switch(iter,end))
		switch (entry.key) {
			case Root::transaction_id: {
				json_read_string(entry.have,transaction_id,"\"transaction_id\"",iter,end);
			} break;
			case Root::platform: {
				json_read_string(entry.have,platform,"\"platform\"",iter,end);
			} break;
			case Root::artworks: {
				if (entry.have != ObjHave::ARRAY)
					return "\"artworks\" must be an array.";
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"artworks\" elements must be objects.";
					
					new_artwork artwork(iter,end);
					
					if (artwork.art_source.empty())
						return "\"artworks\" objects must have a \"source\".";
					if (artwork.data.empty())
						return "\"artworks\" objects must have \"data\".";
					
					new_artworks.emplace(artwork.art_source,artwork);
				}
			} break;
			case Root::accounts: {
				if (entry.have != ObjHave::ARRAY)
					return "\"accounts\" must be an array.";
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"accounts\" elements must be objects.";
					
					new_account account(iter,end);
					
					if (account.acc_platid.empty())
						return "\"accounts\" objects must have an \"id\".";
					if (account.acc_url.empty())
						return "\"accounts\" objects must have an \"url\".";
					
					new_accounts.emplace(account.acc_platid,account);
				};
			} break;
			case Root::tags: {
				if (entry.have != ObjHave::ARRAY)
					return "\"tags\" must be an array.";
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"tags\" elements must be objects.";
					
					new_tag tag(iter,end);
					
					if (tag.tag_platid.empty())
						return "\"tags\" objects must have an \"id\".";
					
					new_tags.emplace(tag.tag_platid,tag);
				};
			} break;
			case Root::art_acc_links: {
				if (entry.have != ObjHave::ARRAY)
					return "\"art_acc_links\" must be an array.";
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"art_acc_links\" elements must be objects.";
						
					auto &link = new_art_acc_links.emplace_back(iter,end);
					
					if (link.acc_platid.empty())
						return "\"art_acc_links\" objects must have a \"tag\".";
					if (link.art_source.empty())
						return "\"art_acc_links\" objects must have an \"artwork\".";
				};
			} break;
			case Root::art_tag_links: {
				if (entry.have != ObjHave::ARRAY)
					return "\"art_tag_links\" must be an array.";
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"art_tag_links\" elements must be objects.";
						
					auto &link = new_art_tag_links.emplace_back(iter,end);
					
					if (link.art_source.empty())
						return "\"art_acc_links\" objects must have an \"artwork\".";
					if (link.tag_platid.empty())
						return "\"art_acc_links\" objects must have a \"tag\".";
				};
			} break;
		}
	// Debug the transaction
	// TODO Enable that with another flag?
	if (0 && Arcollect::debug.webext_adder) {
		std::cerr << "Platform: " << platform << std::endl;
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
	if (Arcollect::debug.webext_adder)
		std::cerr << "Started SQLite transaction" << std::endl;
	// INSERT INTO artworks
	std::unique_ptr<SQLite3::stmt> insert_stmt;
	if (db->prepare(Arcollect::db::sql::adder_insert_artwork.c_str(),insert_stmt)) {
		std::cerr << "Failed to prepare adder_insert_artwork.sql " << db->errmsg() << std::endl;
		std::exit(1);
	}
	for (auto& artwork : new_artworks) {
		insert_stmt->bind(1,artwork.second.art_title);
		insert_stmt->bind(2,platform);
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
				if (!artwork.second.thumbnail.empty()) {
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
		get_account_stmt->bind(1,platform);
		account.second.acc_platid.bind(get_account_stmt,2);
		switch (get_account_stmt->step()) {
			case SQLITE_ROW: {
				// User already exist, save acc_arcoid
				account.second.acc_arcoid = get_account_stmt->column_int64(0);
			} break;
			case SQLITE_DONE: {
				// User does not exist, create it
				account.second.acc_platid.bind(insert_stmt,1);
				insert_stmt->bind(2,platform);
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
		get_tag_stmt->bind(1,platform);
		tag.second.tag_platid.bind(get_tag_stmt,2);
		switch (get_tag_stmt->step()) {
			case SQLITE_ROW: {
				// User already exist, save tag_arcoid
				tag.second.tag_arcoid = get_tag_stmt->column_int64(0);
			} break;
			case SQLITE_DONE: {
				// User does not exist, create it
				tag.second.tag_platid.bind(insert_stmt,1);
				insert_stmt->bind(2,platform);
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

std::string process_json(char* begin, char* const end)
{
	std::string_view transaction_id;
	// Perform addition
	std::optional<std::string> reason;
	try {
		reason = do_add(begin,end,transaction_id);
	} catch (std::exception &e) {
		reason = std::string(e.what());
	}
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
	if (!transaction_id.empty())
		result_json += ",\"transaction_id\":\"" + Arcollect::json::escape_string(transaction_id) + "\"";
	if (reason) {
		result_json += ",\"reason\":\"" + Arcollect::json::escape_string(reason->c_str()) + "\"";
		std::cerr << "Addition failed: " << *reason << std::endl;
	}
	
	return result_json+"}";
}
