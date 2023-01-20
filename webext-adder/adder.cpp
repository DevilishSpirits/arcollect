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
#include <config.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <optional>
#include <vector>
#include <arcollect-db-comics.hpp>
#include <arcollect-debug.hpp>
#include <arcollect-paths.hpp>
#include <arcollect-sqls.hpp>
#include "json-shared-helpers.hpp"
#include "json_escaper.hpp"
#include "download.hpp"
/** \file webext-adder/adder.cpp
 *  \brief Artwork addition routines
 *
 * This file implement the core of the `webext-adder` and contain the code that
 * parse the JSON and perform the transaction in the database in do_add().
 *
 * It works in 2 steps, first the JSON is parsed into C++ structures (`new_*`),
 * their constructor take the JSON parser iter/end pair and consume the object.
 * 
 * It also prefill many #DBCache, it's a local copy of in-dabatase informations,
 * we don't query the database for now but we does allocates the keys in it.
 *
 * The second stage is the SQL transaction, we lock the database and fetch data
 * in all #DBCache. Next we perform the transaction, during it we fill missing
 * #DBCache informations.
 *
 * This file is fat and some parts are splitted in dedicated files.
 */
using namespace std::literals::string_view_literals;
#include "adder-helpers.hpp"
extern std::unique_ptr<SQLite3::sqlite3> db;
static const std::filesystem::path artworks_target_dir("artworks");
static const std::filesystem::path acc_icon_target_dir("account-avatars");

struct db_comic {
	sqlite_int64 com_arcoid;
	db_comic(SQLite3::sqlite3 &db, SQLite3::stmt &stmt) :
		com_arcoid(stmt.column_int64(0))
	{}
	db_comic(sqlite3_int64 id) :
		com_arcoid(id)
	{}
};
using ComicsDBCache = DBCache<platform_id,db_comic>;

struct db_artwork {
	sqlite_int64 art_artid;
	sqlite_int64 art_flag0;
	sqlite_int64 art_partof;
	sqlite_int64 art_dwnid;
	std::optional<sqlite_int64> art_pageno;
	db_artwork(SQLite3::sqlite3 &db, SQLite3::stmt &stmt) :
		art_artid (stmt.column_int64(0)),
		art_flag0 (stmt.column_int64(1)),
		art_partof(stmt.column_int64(2)),
		art_pageno(stmt.column_opt_int64(3))
	{}
};
struct new_comic_page;
struct may_artwork: public DBCacheOptionalBase<db_artwork> {
	ComicsDBCache::optional_type *art_partof = NULL;
	new_comic_page               *comic_page = NULL;
};
using ArtworksDBCache = DBCache<std::string_view,db_artwork,may_artwork>;
struct new_artwork {
	Arcollect::WebextAdder::Download art_dwnid;
	Arcollect::WebextAdder::Download art_thumbnail;
	std::string_view art_title;
	std::string_view art_desc;
	std::string_view art_source;
	sqlite_int64     art_rating = 0;
	std::string_view art_license;
	sqlite_int64     art_postdate = 0;
	ArtworksDBCache::optional_type& cache;
	
	new_artwork(char*& iter, char* const end, Arcollect::WebextAdder::NetworkSession &network_session)
	: art_dwnid(network_session,artworks_target_dir), art_thumbnail(network_session,artworks_target_dir), cache(ArtworksDBCache::dummy())
	{
		enum class Artworks {
			title,
			desc,
			source,
			rating,
			license,
			postdate,
			thumbnail,
			data,
		};
		static const ForEachObjectSwitch<Artworks> artworks_switch{
			{"title"    ,Artworks::title},
			{"desc"     ,Artworks::desc},
			{"source"   ,Artworks::source},
			{"rating"   ,Artworks::rating},
			{"license"  ,Artworks::license},
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
				case Artworks::license: {
					json_read_string(entry.have,art_license,"\"artworks\":[{\"license\"",iter,end);
				} break;
				case Artworks::postdate: {
					art_postdate = json_read_int(entry.have,"\"artworks\":[{\"postdate\"",iter,end);
				} break;
				case Artworks::thumbnail: {
					art_thumbnail.parse(iter,end,entry.have);
				} break;
				case Artworks::data: {
					art_dwnid.parse(iter,end,entry.have);
				} break;
		}
		if (art_source.empty())
			throw std::runtime_error("\"artworks\" objects must have a \"source\".");
		if (art_dwnid.empty())
			throw std::runtime_error("\"artworks\" objects must have \"data\".");
	}
	new_artwork(new_artwork&& other, ArtworksDBCache &db_cache) :
		art_dwnid(std::move(other.art_dwnid)),
		art_thumbnail(std::move(other.art_thumbnail)),
		art_title(std::move(other.art_title)),
		art_desc(std::move(other.art_desc)),
		art_source(std::move(other.art_source)),
		art_rating(std::move(other.art_rating)),
		art_license(std::move(other.art_license)),
		art_postdate(std::move(other.art_postdate)),
		cache(db_cache[other.art_source])
		{}
};

struct db_account {
	sqlite_int64 acc_arcoid;
	db_account(SQLite3::sqlite3 &db, SQLite3::stmt &stmt) :
		acc_arcoid(stmt.column_int64(0))
	{}
};
using AccountsDBCache = DBCache<platform_id,db_account>;
struct new_account {
	platform_id acc_platid;
	Arcollect::WebextAdder::Download acc_icon;
	std::string_view acc_name;
	std::string_view acc_title;
	std::string_view acc_desc;
	std::string_view acc_url;
	std::string_view acc_moneyurl;
	sqlite_int64 acc_createdate = 0;
	AccountsDBCache::optional_type& cache;
	
	new_account(char*& iter, char* const end, Arcollect::WebextAdder::NetworkSession &network_session) : acc_icon(network_session,acc_icon_target_dir), cache(AccountsDBCache::dummy()) {
		enum class Accounts {
			id,
			name,
			title,
			desc,
			url,
			moneyurl,
			icon,
			createdate,
		};
		static const ForEachObjectSwitch<Accounts> accounts_switch{
			{"id"        ,Accounts::id},
			{"name"      ,Accounts::name},
			{"title"     ,Accounts::title},
			{"desc"      ,Accounts::desc},
			{"url"       ,Accounts::url},
			{"moneyurl"  ,Accounts::moneyurl},
			{"icon"      ,Accounts::icon},
			{"createdate",Accounts::createdate},
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
				case Accounts::desc: {
					json_read_string(entry.have,acc_desc,"\"accounts\":[{\"desc\"",iter,end);
				} break;
				case Accounts::url: {
					json_read_string_nul_terminate(entry.have,acc_url,"\"accounts\":[{\"url\"",iter,end);
				} break;
				case Accounts::moneyurl: {
					json_read_string(entry.have,acc_moneyurl,"\"accounts\":[{\"moneyurl\"",iter,end);
				} break;
				case Accounts::icon: {
					acc_icon.parse(iter,end,entry.have);
				} break;
				case Accounts::createdate: {
					acc_createdate = json_read_int(entry.have,"\"accounts\":[{\"createdate\"",iter,end);
				} break;
			}
		if (acc_platid.empty())
			throw std::runtime_error("\"accounts\" objects must have an \"id\".");
		if (acc_url.empty())
			throw std::runtime_error("\"accounts\" objects must have an \"url\".");
		if (acc_icon.empty())
			throw std::runtime_error("\"accounts\" objects must have an \"icon\".");
	}
	new_account(new_account&& other, AccountsDBCache &db_cache) :
		acc_platid(std::move(other.acc_platid)),
		acc_icon(std::move(other.acc_icon)),
		acc_name(std::move(other.acc_name)),
		acc_title(std::move(other.acc_title)),
		acc_desc(std::move(other.acc_desc)),
		acc_url(std::move(other.acc_url)),
		acc_moneyurl(std::move(other.acc_moneyurl)),
		acc_createdate(std::move(other.acc_createdate)),
		cache(db_cache[other.acc_platid])
		{}
};

struct db_tag {
	sqlite_int64 tag_arcoid;
	db_tag(SQLite3::sqlite3 &db, SQLite3::stmt &stmt) :
		tag_arcoid(stmt.column_int64(0))
	{}
};
using TagsDBCache = DBCache<platform_id,db_tag>;
struct new_tag {
	platform_id      tag_platid;
	std::string_view tag_title;
	std::string_view tag_kind;
	sqlite3_int64 tag_createdate = 0;
	TagsDBCache::optional_type& cache;
	
	new_tag(char*& iter, char* const end) : cache(TagsDBCache::dummy()) {
		enum class Tags {
			id,
			title,
			kind,
			createdate,
		};
		static const ForEachObjectSwitch<Tags> tags_switch{
			{"id"          ,Tags::id},
			{"title"      ,Tags::title},
			{"kind"       ,Tags::kind},
			{"createdate" ,Tags::createdate},
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
				case Tags::createdate: {
					tag_createdate = json_read_int(entry.have,"\"tags\":[{\"createdate\"",iter,end);
				} break;
			}
		if (tag_platid.empty())
			throw std::runtime_error("\"tags\" objects must have an \"id\".");
	}
	new_tag(new_tag&& other, TagsDBCache &db_cache) :
		tag_platid(std::move(other.tag_platid)),
		tag_title(std::move(other.tag_title)),
		tag_kind(std::move(other.tag_kind)),
		cache(db_cache[other.tag_platid])
		{}
};

struct new_comic;
struct new_comic_page {
	new_comic *comic;
	ArtworksDBCache::optional_type* relative_to = NULL;
	sqlite3_int64 offset = 0;
	/** Parse a comic page
	 *
	 * This create a new page from the submitted JSON
	 */
	new_comic_page(char*& iter, char* const end, ArtworksDBCache& db_artworks) {
		enum class ComicPage {
			relative_to,
			page,
			sub,
		};
		static const ForEachObjectSwitch<ComicPage> comic_page{
			{"relative_to",ComicPage::relative_to},
			{"page"       ,ComicPage::page},
			{"sub"        ,ComicPage::sub},
		};
		for (auto entry: comic_page(iter,end))
			switch (entry.key) {
				case ComicPage::relative_to: {
					std::string_view relative_to_str;
					json_read_string(entry.have,relative_to_str,"\"comics\":[{\"pages\":{\"...\":{\"relative_to\"",iter,end);
					// Check if relative_to is a part name
					std::optional<Arcollect::db::comics::Part> delta = Arcollect::db::comics::part_from_string(relative_to_str);
					if (delta)
						offset += static_cast<sqlite3_int64>(*delta);
					else relative_to = &db_artworks[relative_to_str];
				} break;
				case ComicPage::page: {
					offset += json_read_int(entry.have,"\"comics\":[{\"pages\":{\"...\":{\"page\"",iter,end)*(1<<Arcollect::db::comics::pageno_shift);
				} break;
				case ComicPage::sub: {
					offset += json_read_int(entry.have,"\"comics\":[{\"pages\":{\"...\":{\"sub\"",iter,end);
				} break;
			}
		if (relative_to && !offset)
			// This is used for a special case
			throw std::runtime_error("An artwork cannot be relative to another one and be at the exact same page");
	}
	/** Create a comic page from a reference in "relative_to"
	 */
	new_comic_page(ArtworksDBCache::optional_type &artwork, new_comic* in_comic) : comic(in_comic), relative_to(&artwork) {}
};
struct new_comic {
	platform_id      com_platid;
	std::string_view com_title;
	std::string_view com_url;
	sqlite3_int64    com_postdate = 0;
	
	std::unordered_map<ArtworksDBCache::optional_type*,new_comic_page> pages;
	
	ComicsDBCache::optional_type  local_cache;
	ComicsDBCache::optional_type *cache;
	
	void move_references(decltype(pages)& pages) {
		for (auto& page: pages) {
			page.second.comic = this;
			page.first->art_partof = cache;
		}
	}
	void merge_from(new_comic& comic) {
		// Merge comics metadatas
		if (!comic.com_platid.empty()) {
			com_platid   = comic.com_platid;
			com_title    = comic.com_title;
			com_url      = comic.com_url;
			com_postdate = comic.com_postdate;
			cache        = comic.cache;
		}
		// Merge pages
		move_references(comic.pages);
		pages.merge(comic.pages);
	}
	
	void merge_to(std::unordered_set<new_comic*> &merge_with, std::vector<new_comic> &new_comics) {
		// Merge comics
		auto iter = merge_with.begin();
		new_comic& target_comic = **iter;
		target_comic.merge_from(*this);
		do {
			target_comic.merge_from(**iter);
		} while (++iter != merge_with.end());
		/* Remove merged comics from new_comics
		 *
		 * We remove elements by erasing a valid comic onto them, order
		 * in the vector does not matter and this avoid jungling with
		 * dangling pointers and more hazardous stuff in addition of better
		 * performances.
		 */
		merge_with.erase(&target_comic);
		while (!merge_with.empty()) {
			// Shrink items located in the back
			std::unordered_set<new_comic*>::iterator iter;
			while ((iter = merge_with.find(&new_comics.back())) != merge_with.end()) {
				new_comics.pop_back();
				merge_with.erase(iter);
			}
			if (merge_with.empty())
				break;
			// Erase item
			iter = merge_with.begin();
			**iter = std::move(new_comics.back());
			new_comics.pop_back();
			merge_with.erase(iter);
		}
	}
	
	/** Parse a comic JSON
	 * \param[out] merge_with The list of comics to merge with.
	 *
	 * Pages in the comic might be inside another comic, this case happen in
	 * platform testset at least.
	 */
	new_comic(char*& iter, char* const end, ComicsDBCache &db_cache, ArtworksDBCache &db_artworks, std::unordered_set<new_comic*> &merge_with) : cache(&local_cache) {
		enum class Comics {
			id,
			title,
			url,
			pages,
			postdate,
		};
		static const ForEachObjectSwitch<Comics> comics_switch{
			{"id"      ,Comics::id},
			{"title"   ,Comics::title},
			{"url"     ,Comics::url},
			{"pages"   ,Comics::pages},
			{"postdate",Comics::postdate},
		};
		for (auto entry: comics_switch(iter,end))
			switch (entry.key) {
				case Comics::id: {
					read_platform_id(entry.have,com_platid,"\"comics\":[{\"id\"",iter,end);
					cache = &db_cache[com_platid];
				} break;
				case Comics::title: {
					json_read_string(entry.have,com_title,"\"comics\":[{\"title\"",iter,end);
				} break;
				case Comics::url: {
					json_read_string(entry.have,com_url,"\"comics\":[{\"url\"",iter,end);
				} break;
				case Comics::pages: {
					std::string_view art_source;
					bool more_pages = true;
					while (more_pages)
						switch (Arcollect::json::read_object_keyval(art_source,iter,end)) {
							case Arcollect::json::ObjHave::OBJECT: {
								// Parse the page
								new_comic_page new_page(iter,end,db_artworks);
								new_page.comic = this;
								if (new_page.relative_to) {
									// The page is relative to another artwork, include it in the comic
									ArtworksDBCache::optional_type &artwork = *new_page.relative_to;
									if (artwork.comic_page) {
										// Already have comic data
										if (artwork.comic_page->comic != this)
											merge_with.insert(artwork.comic_page->comic);
									} else {
										artwork.comic_page = &pages.try_emplace(&artwork,artwork,this).first->second;
									}	
								}
								// Check if the artwork already have a comic page
								ArtworksDBCache::optional_type &artwork = db_artworks[art_source];
								if (artwork.comic_page) {
									// Artwork is already in a comic
									if (artwork.comic_page->comic != this)
										merge_with.insert(artwork.comic_page->comic);
									// Sanity checks and updates
									if (!new_page.relative_to && artwork.comic_page->relative_to) {
										// Our page is absolute, the other is not -> put our version
										artwork.comic_page->relative_to = NULL;
										artwork.comic_page->offset = new_page.offset;
									} else if (new_page.relative_to && artwork.comic_page->relative_to && (!artwork.comic_page->offset)) {
										// This is a page referencing itself -> put our version
										artwork.comic_page->relative_to = new_page.relative_to;
										artwork.comic_page->offset      = new_page.offset;
									} else if (!new_page.relative_to && !artwork.comic_page->relative_to && (new_page.offset != artwork.comic_page->offset))
										// Our page is absolute, the other is too but not the same -> error
										throw std::runtime_error("Page number conflict on "+std::string(art_source)+".");
								} else artwork.comic_page = &pages.emplace(&artwork,std::move(new_page)).first->second;
							} break;
							default: {
							} throw std::runtime_error("Syntax error in \"comics\":[{\"pages\":{ object, expected another object or the end the the \"pages\" object.");
							case Arcollect::json::ObjHave::OBJECT_CLOSE: {
								more_pages = false;
							} break;
						}
				} break;
				case Comics::postdate: {
					com_postdate = json_read_int(entry.have,"\"comics\":[{\"postdate\"",iter,end);
				} break;
			}
	}
	
	new_comic& operator=(new_comic&& other) {
		com_platid = std::move(other.com_platid);
		com_title = std::move(other.com_title);
		com_url = std::move(other.com_url);
		com_postdate = std::move(other.com_postdate);
		pages = std::move(other.pages);
		if (other.cache == &other.local_cache) {
			local_cache = std::move(other.local_cache);
			cache = &local_cache;
		} else cache = std::move(other.cache);
		move_references(pages);
		return *this;
	}
	new_comic(new_comic&& other) {
		operator=(std::forward<new_comic>(other));
	}
	
};

template<typename DBCacheT, typename ParamClass>
struct new_xxx_acc_link {
	typename DBCacheT::optional_type *xxx_id = NULL;
	AccountsDBCache::optional_type *acc_arcoid = NULL;
	std::string_view artacc_link;
	
	new_xxx_acc_link(char*& iter, char* const end, DBCacheT &db_cache, AccountsDBCache &db_accounts) {
		enum class AccLinks {
			xxx,
			account,
			link,
		};
		static const ForEachObjectSwitch<AccLinks> xxx_acc_links_switch{
			{ParamClass::linked_field,AccLinks::xxx},
			{"account",AccLinks::account},
			{"link"   ,AccLinks::link},
		};
		for (auto entry: xxx_acc_links_switch(iter,end))
			switch (entry.key) {
				case AccLinks::xxx: {
					xxx_id = &db_cache.read_json_ref(entry.have,("\""+std::string(ParamClass::arr_name)+"\":[{\""+std::string(ParamClass::linked_field)+"\"").c_str(),iter,end);
				} break;
				case AccLinks::account: {
					platform_id acc_platid;
					read_platform_id(entry.have,acc_platid,"\""+std::string(ParamClass::arr_name)+"\":[{\"account\"",iter,end);
					acc_arcoid = &db_accounts[acc_platid];
				} break;
				case AccLinks::link: {
					json_read_string(entry.have,artacc_link,"\""+std::string(ParamClass::arr_name)+"\":[{\"link\"",iter,end);
				} break;
			}
		// Sanity checks
		if (!xxx_id)
			throw std::runtime_error("\""+std::string(ParamClass::arr_name)+"\" objects must have a \""+std::string(ParamClass::linked_field)+"\".");
		if (!acc_arcoid)
			throw std::runtime_error("\""+std::string(ParamClass::arr_name)+"\" objects must have a \"tag\".");
	}
	/** Check if the link is sane
	 * \return true if the link is sane
	 *
	 * A link is sane if both *xxx_id and *acc_arcoid optional are usable.
	 * In short, you can **xxx_id and **acc_arcoid without fear of UB.
	 */
	operator bool(void) const {
		return *xxx_id && *acc_arcoid;
	}
};
#define DEFINE_new_xxx_acc_link(link_type,DBCacheT,linked_field_str) \
	struct link_type##ParamClass{ static constexpr std::string_view arr_name = #link_type "s"; static constexpr std::string_view linked_field = linked_field_str;}; \
	using new_##link_type = new_xxx_acc_link<DBCacheT,link_type##ParamClass>
DEFINE_new_xxx_acc_link(art_acc_link,ArtworksDBCache,"artwork");
DEFINE_new_xxx_acc_link(com_acc_link,ComicsDBCache,"comic");

template<typename DBCacheT, typename ParamClass>
struct new_xxx_tag_link {
	typename DBCacheT::optional_type *xxx_id = NULL;
	TagsDBCache::optional_type *tag_arcoid = NULL;
	
	new_xxx_tag_link(char*& iter, char* const end, DBCacheT &db_cache, TagsDBCache &db_tags) {
		enum class TagLinks {
			artwork,
			tag,
		};
		static const ForEachObjectSwitch<TagLinks> xxx_tag_links_switch{
			{ParamClass::linked_field,TagLinks::artwork},
			{"tag"    ,TagLinks::tag},
		};
		for (auto entry: xxx_tag_links_switch(iter,end))
			switch (entry.key) {
				case TagLinks::artwork: {
					xxx_id = &db_cache.read_json_ref(entry.have,("\""+std::string(ParamClass::arr_name)+"\":[{\""+std::string(ParamClass::linked_field)+"\"").c_str(),iter,end);
				} break;
				case TagLinks::tag: {
					platform_id tag_platid;
					read_platform_id(entry.have,tag_platid,("\""+std::string(ParamClass::arr_name)+"\":[{\"tag\"").c_str(),iter,end);
					tag_arcoid = &db_tags[tag_platid];
				} break;
			}
		// Sanity checks
		if (!xxx_id)
			std::runtime_error("\""+std::string(ParamClass::arr_name)+"\" objects must have a \""+std::string(ParamClass::linked_field)+"\".");
		if (!tag_arcoid)
			std::runtime_error("\""+std::string(ParamClass::arr_name)+"\" objects must have a \"tag\".");
	}
	/** Check if the link is sane
	 * \return true if the link is sane
	 *
	 * A link is sane if both *xxx_id and *tag_arcoid optional are usable.
	 * In short, you can **xxx_id and **tag_arcoid without fear of UB.
	 */
	operator bool(void) const {
		return *xxx_id && *tag_arcoid;
	}
};
#define DEFINE_new_xxx_tag_link(link_type,DBCacheT,linked_field_str) \
	struct link_type##ParamClass{ static constexpr std::string_view arr_name = #link_type "s"; static constexpr std::string_view linked_field = linked_field_str;}; \
	using new_##link_type = new_xxx_tag_link<DBCacheT,link_type##ParamClass>
DEFINE_new_xxx_tag_link(art_tag_link,ArtworksDBCache,"artwork");
DEFINE_new_xxx_tag_link(com_tag_link,ComicsDBCache,"comic");

static sqlite_int64 alloc_comicid(SQLite3::sqlite3 &db, SQLite3::stmt &alloc_comicid_stmt) {
	sqlite3_int64 result;
	switch (alloc_comicid_stmt.step()) {
		case SQLITE_ROW: {
			result = alloc_comicid_stmt.column_int64(0);
			if (alloc_comicid_stmt.step() != SQLITE_DONE)
				throw std::runtime_error(std::string("Failed to allocate a comic id (in SQLITE_ROW handling): "+std::string(db.errmsg())));
		} break;
		case SQLITE_DONE: {
		} throw std::runtime_error("Comic id allocation returned no row. What?!");
		default: {
		} throw std::runtime_error("Failed to allocate a comic id: "+std::string(db.errmsg()));
	}
	alloc_comicid_stmt.reset();
	return result;
}
static std::optional<std::string> do_add(char* iter, char* const end, std::string_view &transaction_id, Arcollect::db::downloads::Transaction &dwn_transaction)
{
	using namespace Arcollect::json;
	Arcollect::WebextAdder::NetworkSession network_session(dwn_transaction);
	std::string_view platform;
	ArtworksDBCache db_artworks;
	AccountsDBCache db_accounts;
	TagsDBCache     db_tags;
	ComicsDBCache   db_comics;
	std::vector<new_artwork> new_artworks;
	std::vector<new_account> new_accounts;
	std::vector<new_tag>     new_tags;
	std::vector<new_comic>   new_comics;
	std::vector<new_art_acc_link> new_art_acc_links;
	std::vector<new_art_tag_link> new_art_tag_links;
	std::vector<new_com_acc_link> new_com_acc_links;
	std::vector<new_com_tag_link> new_com_tag_links;
	std::unordered_map<std::string_view,decltype(new_comics)::size_type> art_comic_pages; // Art to comic pages links
	if (Arcollect::debug.webext_adder)
		std::cerr << "Parsing JSON..." << std::endl;
	// Read root
	if (what_i_have(iter,end) != Have::OBJECT)
		return "Invalid JSON, expected a root object.";
	
	// Parse JSONs
	enum class Root {
		transaction_id,
		platform,
		referrer_policy,
		dns_prefill,
		artworks,
		accounts,
		tags,
		comics,
		art_acc_links,
		art_tag_links,
		com_tag_links,
		com_acc_links,
	};
	static const ForEachObjectSwitch<Root> root_switch{
		{"transaction_id",Root::transaction_id},
		{"platform"      ,Root::platform},
		{"referrer_policy",Root::referrer_policy},
		{"dns_prefill"   ,Root::dns_prefill},
		{"artworks"      ,Root::artworks},
		{"accounts"      ,Root::accounts},
		{"tags"          ,Root::tags},
		{"comics"        ,Root::comics},
		{"art_acc_links" ,Root::art_acc_links},
		{"art_tag_links" ,Root::art_tag_links},
		{"com_acc_links" ,Root::com_acc_links},
		{"com_tag_links" ,Root::com_tag_links},
	};
	for (auto entry: root_switch(iter,end))
		switch (entry.key) {
			case Root::transaction_id: {
				json_read_string(entry.have,transaction_id,"\"transaction_id\"",iter,end);
			} break;
			case Root::platform: {
				json_read_string(entry.have,platform,"\"platform\"",iter,end);
				if (Arcollect::debug.webext_adder)
					std::cerr << "\tPlatform: " << platform << std::endl;
			} break;
			case Root::referrer_policy: {
				std::string_view referrer_policy_string;
				json_read_string(entry.have,referrer_policy_string,"\"referrer_policy\"",iter,end);
				network_session.referrer_policy = Arcollect::WebextAdder::parse_referrer_policy(referrer_policy_string);
			} break;
			case Root::dns_prefill: {
				if (entry.have != Arcollect::json::ObjHave::OBJECT)
					throw std::runtime_error("\"dns_prefill\": must be an object of arrays.");
				std::string_view hostname;
				bool has_hostname = true;
				while (has_hostname) {
					switch (Arcollect::json::read_object_keyval(hostname,iter,end)) {
						case Arcollect::json::ObjHave::ARRAY: {
							std::string new_entry("+");
							new_entry += hostname;
							new_entry += ":443:"; // Assume HTTPS.
							for (ArrHave have: Arcollect::json::Array(iter,end)) {
								std::string_view addr;
								json_read_string(static_cast<Arcollect::json::ObjHave>(have),addr,"\"dns_prefill\":{[\"addr...\"",iter,end);
								new_entry += addr;
								new_entry.push_back(',');
							}
							if (new_entry.back() != ',')
								continue; // No address found, skip this entry
							new_entry.pop_back();
							network_session.dns_prefill.append(new_entry.c_str());
							if (Arcollect::debug.webext_adder)
								std::cerr << "DNS prefill: " << new_entry << std::endl;
						} break;
						case Arcollect::json::ObjHave::OBJECT_CLOSE: {
							has_hostname = false;
						} break;
						default: {
							throw std::runtime_error("\"dns_prefill\">:{ elements must be arrays of strings.");
						} break;
					}
				}
				curl_easy_setopt(network_session.easyhandle,CURLOPT_RESOLVE,network_session.dns_prefill.list); // Note that the cache survive curl_easy_reset() so we set it once there.
			} break;
			case Root::artworks: {
				if (entry.have != ObjHave::ARRAY)
					return "\"artworks\" must be an array.";
				if (Arcollect::debug.webext_adder)
					std::cerr << "\tProcessing \"artworks\"..." << std::endl;
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"artworks\" elements must be objects.";
					new_artworks.emplace_back(new_artwork(iter,end,network_session),db_artworks);
				}
			} break;
			case Root::accounts: {
				if (entry.have != ObjHave::ARRAY)
					return "\"accounts\" must be an array.";
				if (Arcollect::debug.webext_adder)
					std::cerr << "\tProcessing \"accounts\"..." << std::endl;
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"accounts\" elements must be objects.";
					new_accounts.emplace_back(new_account(iter,end,network_session),db_accounts);
				};
			} break;
			case Root::tags: {
				if (entry.have != ObjHave::ARRAY)
					return "\"tags\" must be an array.";
				if (Arcollect::debug.webext_adder)
					std::cerr << "\tProcessing \"tags\"..." << std::endl;
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"tags\" elements must be objects.";
					new_tags.emplace_back(new_tag(iter,end),db_tags);
				};
			} break;
			case Root::comics: {
				if (entry.have != ObjHave::ARRAY)
					return "\"comics\" must be an array.";
				if (Arcollect::debug.webext_adder)
					std::cerr << "\tProcessing \"comics\"..." << std::endl;
				for (ArrHave have: Arcollect::json::Array(iter,end)) {
					if (have != ArrHave::OBJECT)
						return "\"comics\" elements must be objects.";
					std::unordered_set<new_comic*> merge_with;
					new_comic comic(iter,end,db_comics,db_artworks,merge_with);
					if (merge_with.empty())
						// New comic, simply append it
						new_comics.emplace_back(std::move(comic));
					else comic.merge_to(merge_with,new_comics);
				}
			} break;
			case Root::art_acc_links: {
				parse_links("art_acc_links",entry.have,iter,end,new_art_acc_links,db_artworks,db_accounts);
			} break;
			case Root::art_tag_links: {
				parse_links("art_tag_links",entry.have,iter,end,new_art_tag_links,db_artworks,db_tags);
			} break;
			case Root::com_acc_links: {
				parse_links("com_acc_links",entry.have,iter,end,new_com_acc_links,db_comics,db_accounts);
			} break;
			case Root::com_tag_links: {
				parse_links("com_tag_links",entry.have,iter,end,new_com_tag_links,db_comics,db_tags);
			} break;
		}
	// Debug the transaction
	// TODO Enable that with another flag?
	if (0 && Arcollect::debug.webext_adder) {
		std::cerr << "Platform: " << platform << std::endl;
		std::cerr << new_artworks.size() << " artwork(s) :" << std::endl;
		for (auto& artwork : new_artworks)
			std::cerr << "\t\"" << artwork.art_title << "\" (" << artwork.art_source << ")\n" << std::endl;
		std::cerr << new_accounts.size() << " account(s) :" << std::endl;
		for (auto& account : new_accounts)
			std::cerr << "\t\"" << account.acc_title << "\" (" << account.acc_name << ") [" << account.acc_platid << "]\n" << std::endl;
		std::cerr << new_art_acc_links.size() << " artwork/account link(s) :" << std::endl;
		/* TODO
		for (auto& art_acc_link : new_art_acc_links)
			std::cerr << "\t\"" << art_acc_link. << "\" (" << account..acc_name << ") [" << (account..acc_platid_str ? account..acc_platid_str : std::to_string(account..acc_platid_int).c_str()) << "]" << std::endl << std::endl;
		*/
	}
	
	/*** Stage 2 - SQLite transaction ***/
	// Begin transaction
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
		
	// Read cache
	std::unique_ptr<SQLite3::stmt> adder_cache_stmt;
	if (db->prepare(Arcollect::db::sql::adder_cache_artwork,adder_cache_stmt))
		return "Failed to prepare adder_cache_artwork " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tLooking for " << db_artworks.cache_map.size() << " artworks..." << std::endl;
	db_artworks.query_db(*db,*adder_cache_stmt,platform);
	
	if (db->prepare(Arcollect::db::sql::adder_cache_account,adder_cache_stmt))
		return "Failed to prepare adder_cache_account " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tLooking for " << db_accounts.cache_map.size() << " accounts..." << std::endl;
	db_accounts.query_db(*db,*adder_cache_stmt,platform);
	
	if (db->prepare(Arcollect::db::sql::adder_cache_comic,adder_cache_stmt))
		return "Failed to prepare adder_cache_comic " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tLooking for " << db_comics.cache_map.size() << " comics..." << std::endl;
	db_comics.query_db(*db,*adder_cache_stmt,platform);
	
	if (db->prepare(Arcollect::db::sql::adder_cache_tag,adder_cache_stmt))
		return "Failed to prepare adder_cache_tag " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tLooking for " << db_tags.cache_map.size() << " tags..." << std::endl;
	db_tags.query_db(*db,*adder_cache_stmt,platform);
	
	// TODO Read the comics_missing_pages table
	
	// Now write in the database!
	std::unique_ptr<SQLite3::stmt>  insert_stmt;
	std::unique_ptr<SQLite3::stmt>  update_stmt;
	std::unique_ptr<SQLite3::stmt>  alloc_comicid_stmt;
	
	// INSERT INTO comics
	if (db->prepare(Arcollect::db::sql::adder_insert_comic,insert_stmt))
		return "Failed to prepare adder_insert_comic: " + std::string(db->errmsg());
	if (db->prepare(Arcollect::db::sql::adder_update_comic,update_stmt))
		return "Failed to prepare adder_update_comic: " + std::string(db->errmsg());
	if (db->prepare(Arcollect::db::sql::allocate_comicid,alloc_comicid_stmt))
		return "Failed to prepare allocate_comicid: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_comics.size() << " comics..." << std::endl;
	for (auto& comic : new_comics) {
		// Resolve some artworks from the 
		for (auto& page: comic.pages)
			if ((page.first == page.second.relative_to) // The page has a unresolved reference that may be in the DB
			&& (*page.first)                       // The page is the database
			&& ((*page.first)->art_pageno)) { // The artwork have a page number
				page.second.relative_to = NULL;
				page.second.offset      = *(*page.first)->art_pageno;
			}
		// Resolve links
		// TODO I don't like the O(n²) complexity
		for (const auto& page: comic.pages)
			for (auto& page2: comic.pages)
				if (page2.second.relative_to == page.first) {
					// Update the page
					page2.second.relative_to = page.second.relative_to;
					page2.second.offset     += page.second.offset;
				}
		/* Note! Some platform doesn't support comics but the webextension may find
		 * out that the account wanted to make a comic, it creates a pseudo-comic in
		 * the artworks table but nothing in the "comics" table, so no com_platid.
		 * This is why we make the check just below.
		 */
		auto &cache = *comic.cache;
		if (!comic.com_platid.empty()) {
			// Perform SQL
			if (cache) {
				// Update
				update_stmt->bind(1,cache->com_arcoid);
				update_stmt->bind(2,comic.com_title);
				update_stmt->bind(3,comic.com_url);
				bind_timestamp(update_stmt,4,comic.com_postdate);
				switch (update_stmt->step()) {
					case SQLITE_ROW: {
						cache.emplace(db_comic(*db,*update_stmt));
						if (update_stmt->step() != SQLITE_DONE)
							return std::string("Failed to update comic (in SQLITE_ROW handling): "+std::string(db->errmsg()));
					} break;
					case SQLITE_DONE: {
					} return "comic update returned no row. What?!";
					default: {
					} return "Failed to update comic: "+std::string(db->errmsg());
				}	
				update_stmt->reset();
			} else {
				// Insert
				comic.com_platid.bind(insert_stmt,1);
				insert_stmt->bind(2,platform);
				insert_stmt->bind(3,comic.com_title);
				insert_stmt->bind(4,comic.com_url);
				bind_timestamp(insert_stmt,5,comic.com_postdate);
				switch (insert_stmt->step()) {
					case SQLITE_ROW: {
						cache.emplace(*db,*insert_stmt);
						if (insert_stmt->step() != SQLITE_DONE)
							return std::string("Failed to insert comic (in SQLITE_ROW handling): "+std::string(db->errmsg()));
					} break;
					case SQLITE_DONE: {
					} return "Comic insertion returned no row. What?!";
					default: {
					} return "Failed to insert comic: "+std::string(db->errmsg());
				}
				insert_stmt->reset();
			}
		} else {
			// Entry-less comic
			// Find if an artwork is already in a comic
			for (const auto& page: comic.pages)
				if (*page.first) {
					cache.emplace((*page.first)->art_partof);
					break;
				}
			// Allocate an id for new comics
			if (!cache)
				cache.emplace(alloc_comicid(*db,*alloc_comicid_stmt));
		}
	}
	// INSERT INTO artworks
	if (db->prepare(Arcollect::db::sql::adder_insert_artwork,insert_stmt))
		return "Failed to prepare adder_insert_artwork: " + std::string(db->errmsg());
	if (db->prepare(Arcollect::db::sql::adder_update_artwork,update_stmt))
		return "Failed to prepare adder_update_artwork: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_artworks.size() << " artworks..." << std::endl;
	for (auto& artwork : new_artworks) {
		// Mark our artwork as handlede for the comic page updater
		const new_comic_page* comic_page = artwork.cache.comic_page;
		artwork.cache.comic_page = NULL;
		// Skip if the artwork is frozen
		if (artwork.cache && (artwork.cache->art_flag0 & 1)) {
			continue;
		}
		// Handle comic related stuff
		sqlite3_int64 art_partof = artwork.cache.art_partof ? (*artwork.cache.art_partof)->com_arcoid : alloc_comicid(*db,*alloc_comicid_stmt);
		if (comic_page && (comic_page->relative_to != NULL))
			// Comic absolute page number is unknow -> don't write it
			comic_page = NULL;
		std::optional<sqlite3_int64> art_pageno = comic_page ? std::make_optional(comic_page->offset) : std::nullopt;
		// Download data
		sqlite3_int64 art_flag0 = artwork.cache ? artwork.cache->art_flag0 : 0;
		std::string filename(platform);
		filename += "_";
		filename += artwork.art_dwnid.base_filename();
		sqlite_int64 dwnid = (art_flag0 & 1)
			? artwork.cache->art_dwnid // Artwork data frozen -> do not change
			: artwork.art_dwnid.perform(filename,artwork.art_source);
		filename += ".thumbnail";
		sqlite_int64 thumbnail = artwork.art_thumbnail.empty() ? dwnid : artwork.art_thumbnail.perform(filename,artwork.art_source);
		// Perform SQL
		if (artwork.cache) {
			update_stmt->bind(1,artwork.cache->art_artid);
			update_stmt->bind(2,dwnid);
			update_stmt->bind(3,thumbnail);
			update_stmt->bind(4,artwork.art_title);
			update_stmt->bind(5,artwork.art_desc);
			update_stmt->bind(6,artwork.art_license);
			update_stmt->bind(7,art_partof);
			update_stmt->bind(8,art_pageno);
			bind_timestamp(update_stmt,9,artwork.art_postdate);
			switch (update_stmt->step()) {
				case SQLITE_ROW: {
					// Save artwork
					artwork.cache.emplace(db_artwork(*db,*update_stmt));
					if (update_stmt->step() != SQLITE_DONE)
						return std::string("Failed to update artwork (in SQLITE_ROW handling): "+std::string(db->errmsg()));
				} break;
				case SQLITE_DONE: {
				} return "Artwork update returned no row. What?!";
				default: {
				} return "Failed to update artwork: "+std::string(db->errmsg());
			}
			update_stmt->reset();
		} else {
			// Insert
			insert_stmt->bind(1,dwnid);
			insert_stmt->bind(2,thumbnail);
			insert_stmt->bind(3,platform);
			insert_stmt->bind(4,artwork.art_title);
			insert_stmt->bind(5,artwork.art_desc);
			insert_stmt->bind(6,artwork.art_source);
			insert_stmt->bind(7,artwork.art_rating);
			insert_stmt->bind(8,artwork.art_license);
			insert_stmt->bind(9,art_partof);
			insert_stmt->bind(10,art_pageno);
			bind_timestamp(insert_stmt,11,artwork.art_postdate);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
					artwork.cache.emplace(db_artwork(*db,*insert_stmt));
					if (insert_stmt->step() != SQLITE_DONE)
						return std::string("Failed to insert artwork (in SQLITE_ROW handling): "+std::string(db->errmsg()));
				} break;
				case SQLITE_DONE: {
				} return "Artwork insertion returned no row. What?!";
				default: {
				} return "Failed to insert artwork: "+std::string(db->errmsg());
			}
			insert_stmt->reset();
		}
	} new_artworks.clear();
	
	// Update other artworks pages if an absolute one is known and the artwork is not frozen
	if (db->prepare(Arcollect::db::sql::adder_update_artwork_comic,update_stmt))
		return "Failed to prepare adder_update_artwork_comic: " + std::string(db->errmsg());
	for (auto& comic : new_comics)
		for (auto& page: comic.pages)
			if (*page.first && !page.second.relative_to && !((*page.first)->art_flag0 & 1)) {
				update_stmt->bind(1,(*page.first)->art_artid);
				update_stmt->bind(2,(*comic.cache)->com_arcoid);
				update_stmt->bind(3,page.second.offset);
				switch (update_stmt->step()) {
					case SQLITE_DONE: {
					} break;
					default: {
					} return "Failed to update artwork comic infos: " + std::string(db->errmsg());
				}
				update_stmt->reset();
			}
	
	// TODO Write back the comics_missing_pages table
	
	// INSERT INTO accounts
	struct new_acc_icon {
		sqlite3_int64 acc_arcoid;
		sqlite3_int64 dwn_id;
		new_acc_icon(sqlite3_int64 acc_arcoid, sqlite3_int64 dwn_id) : acc_arcoid(acc_arcoid), dwn_id(dwn_id) {}
	};
	std::vector<new_acc_icon> new_acc_icons;
	if (db->prepare(Arcollect::db::sql::adder_insert_account,insert_stmt))
		return "Failed to prepare adder_insert_account: " + std::string(db->errmsg());
	if (db->prepare(Arcollect::db::sql::adder_update_account,update_stmt))
		return "Failed to prepare adder_update_account: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_accounts.size() << " accounts..." << std::endl;
	for (auto& account : new_accounts) {
		// Download data
		std::string filename(platform);
		filename += "_";
		filename += account.acc_name;
		sqlite_int64 icon = account.acc_icon.perform(filename,account.acc_url);
		// Perform SQL
		if (account.cache) {
			// Update
			update_stmt->bind(1,account.cache->acc_arcoid);
			update_stmt->bind(2,icon);
			update_stmt->bind(3,account.acc_name);
			update_stmt->bind(4,account.acc_title);
			update_stmt->bind(5,account.acc_desc);
			update_stmt->bind(6,account.acc_url);
			update_stmt->bind(7,account.acc_moneyurl);
			bind_timestamp(update_stmt,8,account.acc_createdate);
			switch (update_stmt->step()) {
				case SQLITE_ROW: {
					account.cache.emplace(db_account(*db,*update_stmt));
					new_acc_icons.emplace_back(update_stmt->column_int64(0),icon);
					if (update_stmt->step() != SQLITE_DONE)
						return std::string("Failed to update account (in SQLITE_ROW handling): "+std::string(db->errmsg()));
				} break;
				case SQLITE_DONE: {
				} return "Account update returned no row. What?!";
				default: {
				} return "Failed to update artwork: "+std::string(db->errmsg());
			}
			update_stmt->reset();
		} else {
			// Insert
			account.acc_platid.bind(insert_stmt,1);
			insert_stmt->bind(2,icon);
			insert_stmt->bind(3,platform);
			insert_stmt->bind(4,account.acc_name);
			insert_stmt->bind(5,account.acc_title);
			insert_stmt->bind(6,account.acc_desc);
			insert_stmt->bind(7,account.acc_url);
			insert_stmt->bind(8,account.acc_moneyurl);
			bind_timestamp(insert_stmt,9,account.acc_createdate);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
					// Save account
					account.cache.emplace(*db,*insert_stmt);
					new_acc_icons.emplace_back(insert_stmt->column_int64(0),icon);
					if (insert_stmt->step() != SQLITE_DONE)
						return std::string("Failed to insert account (in SQLITE_ROW handling): "+std::string(db->errmsg()));
				} break;
				case SQLITE_DONE: {
				} return "Account insertion returned no row. What?! "+std::string(db->errmsg());
				default: {
				} return "Failed to insert account: "+std::string(db->errmsg());
			}
			insert_stmt->reset();
		}
	} new_accounts.clear();
	
	// INSERT INTO tags
	if (db->prepare(Arcollect::db::sql::adder_insert_tag,insert_stmt))
		return "Failed to prepare adder_insert_tag: " + std::string(db->errmsg());
	if (db->prepare(Arcollect::db::sql::adder_update_tag,update_stmt))
		return "Failed to prepare adder_update_tag: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_tags.size() << " tags..." << std::endl;
	for (auto& tag : new_tags) {
		// Perform SQL
		if (tag.cache) {
			// Update
			update_stmt->bind(1,tag.cache->tag_arcoid);
			update_stmt->bind(2,tag.tag_title);
			update_stmt->bind(3,tag.tag_kind);
			bind_timestamp(update_stmt,4,tag.tag_createdate);
			switch (update_stmt->step()) {
				case SQLITE_ROW: {
					tag.cache.emplace(db_tag(*db,*update_stmt));
					if (update_stmt->step() != SQLITE_DONE)
						return std::string("Failed to update tag (in SQLITE_ROW handling): "+std::string(db->errmsg()));
				} break;
				case SQLITE_DONE: {
				} return "Tag update returned no row. What?!";
				default: {
				} return "Failed to update tag: "+std::string(db->errmsg());
			}	
			update_stmt->reset();
		} else {
			// Insert
			tag.tag_platid.bind(insert_stmt,1);
			insert_stmt->bind(2,platform);
			insert_stmt->bind(3,tag.tag_title);
			insert_stmt->bind(4,tag.tag_kind);
			bind_timestamp(insert_stmt,5,tag.tag_createdate);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
					tag.cache.emplace(*db,*insert_stmt);
					if (insert_stmt->step() != SQLITE_DONE)
						return std::string("Failed to insert tag (in SQLITE_ROW handling): "+std::string(db->errmsg()));
				} break;
				case SQLITE_DONE: {
				} return "Tag insertion returned no row. What?!";
				default: {
				} return "Failed to insert tag: "+std::string(db->errmsg());
			}
			insert_stmt->reset();
		}
	} new_tags.clear();
	
	// INSERT INTO art_acc_links
	if (db->prepare(Arcollect::db::sql::adder_insert_art_acc_link,insert_stmt))
		return "Failed to prepare adder_insert_art_acc_link: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_art_acc_links.size() << " artwork/account links..." << std::endl;
	for (const auto&  art_acc_link: new_art_acc_links)
		if (art_acc_link) {
			const sqlite3_int64 art_artid  = (*art_acc_link.xxx_id)->art_artid;
			const sqlite3_int64 acc_arcoid = (*art_acc_link.acc_arcoid)->acc_arcoid;
			insert_stmt->bind(1,art_artid);
			insert_stmt->bind(2,acc_arcoid);
			insert_stmt->bind(3,art_acc_link.artacc_link);
			switch (insert_stmt->step()) {
				case SQLITE_DONE: {
				} break;
				case SQLITE_CONSTRAINT: {
					if (db->extended_errcode() == SQLITE_CONSTRAINT_PRIMARYKEY)
						// Link already exists, it's fine
						break;
				} // falltrough;
				default: {
				} return "Failed to insert artwork/account link: " + std::string(db->errmsg());
			}
			insert_stmt->reset();
		}
	new_art_acc_links.clear();
	
	// INSERT INTO art_tag_links
	if (db->prepare(Arcollect::db::sql::adder_insert_art_tag_link,insert_stmt))
		return "Failed to prepare adder_insert_art_tag_link: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_art_tag_links.size() << " artwork/tag links..." << std::endl;
	for (const auto& art_tag_link : new_art_tag_links)
		if (art_tag_link) {
			const sqlite3_int64 art_artid  = (*art_tag_link.xxx_id)->art_artid;
			const sqlite3_int64 tag_arcoid = (*art_tag_link.tag_arcoid)->tag_arcoid;
			insert_stmt->bind(1,art_artid);
			insert_stmt->bind(2,tag_arcoid);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
				} break;
				case SQLITE_DONE: {
				} break;
					case SQLITE_CONSTRAINT: {
						if (db->extended_errcode() == SQLITE_CONSTRAINT_PRIMARYKEY)
							// Link already exists, it's fine
							break;
					} // falltrough;
				default: {
				} return "Failed to insert artwork/tag link: " + std::string(db->errmsg());
			}
			insert_stmt->reset();
		}
	new_art_tag_links.clear();
	
	// INSERT INTO com_acc_links
	if (db->prepare(Arcollect::db::sql::adder_insert_com_acc_link,insert_stmt))
		return "Failed to prepare adder_insert_com_acc_link: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_com_acc_links.size() << " comic/account links..." << std::endl;
	for (const auto& com_acc_link : new_com_acc_links)
		if (com_acc_link) {
			const sqlite3_int64 com_arcoid = (*com_acc_link.xxx_id)->com_arcoid;
			const sqlite3_int64 acc_arcoid = (*com_acc_link.acc_arcoid)->acc_arcoid;
			insert_stmt->bind(1,com_arcoid);
			insert_stmt->bind(2,acc_arcoid);
			insert_stmt->bind(3,com_acc_link.artacc_link);
			switch (insert_stmt->step()) {
				case SQLITE_DONE: {
				} break;
				case SQLITE_CONSTRAINT: {
					if (db->extended_errcode() == SQLITE_CONSTRAINT_PRIMARYKEY)
						// Link already exists, it's fine
						break;
				} // falltrough;
				default: {
				} return "Failed to insert comic/account link: " + std::string(db->errmsg());
			}
			insert_stmt->reset();
		}
	new_com_acc_links.clear();
	
	// INSERT INTO com_tag_links
	if (db->prepare(Arcollect::db::sql::adder_insert_com_tag_link,insert_stmt))
		return "Failed to prepare adder_insert_com_tag_link: " + std::string(db->errmsg());
	if (Arcollect::debug.webext_adder)
		std::cerr << "\tInserting " << new_com_tag_links.size() << " comic/tag links..." << std::endl;
	for (const auto& com_tag_link : new_com_tag_links)
		if (com_tag_link) {
			const sqlite3_int64 com_arcoid = (*com_tag_link.xxx_id)->com_arcoid;
			const sqlite3_int64 tag_arcoid = (*com_tag_link.tag_arcoid)->tag_arcoid;
			insert_stmt->bind(1,com_arcoid);
			insert_stmt->bind(2,tag_arcoid);
			switch (insert_stmt->step()) {
				case SQLITE_ROW: {
				} break;
				case SQLITE_DONE: {
				} break;
				case SQLITE_CONSTRAINT: {
					if (db->extended_errcode() == SQLITE_CONSTRAINT_PRIMARYKEY)
						// Link already exists, it's fine
						break;
				} // falltrough;
				default: {
				} return "Failed to insert comic/tag link: " + std::string(db->errmsg());
			}
			insert_stmt->reset();
		}
	new_com_tag_links.clear();
	
	// INSERT INTO acc_icons
	if (db->prepare(Arcollect::db::sql::adder_insert_acc_icon,insert_stmt))
		return "Failed to prepare adder_insert_acc_icon: " + std::string(db->errmsg());
	for (auto& acc_icon : new_acc_icons) {
		insert_stmt->bind(1,acc_icon.acc_arcoid);
		insert_stmt->bind(2,acc_icon.dwn_id);
		switch (insert_stmt->step()) {
			case SQLITE_ROW: {
			} break;
			case SQLITE_DONE: {
			} break;
			case SQLITE_CONSTRAINT: {
				if (db->extended_errcode() == SQLITE_CONSTRAINT_PRIMARYKEY)
					// Link already exists, it's fine
					break;
			} // falltrough;
			default: {
			} return "Failed to insert account/icon history: " + std::string(db->errmsg());
		}
		insert_stmt->reset();
	} new_acc_icons.clear();
	// Return success
	return std::nullopt;
}

std::string process_json(char* begin, char* const end)
{
	std::string_view transaction_id;
	// Perform addition
	std::optional<std::string> reason;
	Arcollect::db::downloads::Transaction dwn_transaction(db);
	try {
		reason = do_add(begin,end,transaction_id,dwn_transaction);
	} catch (std::exception &e) {
		reason = std::string(e.what());
	}
	// Commit or rollback
	if (reason)
		db->exec("ROLLBACK;"); // TODO Check errors
	else {
		const char *const default_errmsg =  "Well... SQLite went wrong when commiting.";
		char* errmsg = const_cast<char*>(default_errmsg); // Note: On success, SQLite set it to NULL
		db->exec("COMMIT;",NULL,NULL,&errmsg);
		if (errmsg) {
			reason = std::string(errmsg);
			if (errmsg != default_errmsg)
				sqlite3_free(errmsg);
		} else dwn_transaction.commit();
	}
	
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
