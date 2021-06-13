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
#include "artwork.hpp"
#include "artwork-loader.hpp"
#include "account.hpp"
#include "db.hpp"
#include <arcollect-paths.hpp>
#include <iostream>
#include <SDL_image.h>
static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::artwork>> artworks_pool;

Arcollect::db::artwork::artwork(Arcollect::db::artwork_id art_id) :
	data_version(-2),
	art_id(art_id)
{
}
std::shared_ptr<Arcollect::db::artwork> &Arcollect::db::artwork::query(Arcollect::db::artwork_id art_id)
{
	std::shared_ptr<Arcollect::db::artwork> &pointer = artworks_pool.try_emplace(art_id).first->second;
	if (!pointer)
		pointer = std::shared_ptr<Arcollect::db::artwork>(new Arcollect::db::artwork(art_id));
	return pointer;
}

SDL::Surface *Arcollect::db::artwork::load_surface(void)
{
	const std::string path = Arcollect::path::artwork_pool / std::to_string(art_id);
	return (SDL::Surface*)IMG_Load(path.c_str());
}
std::unique_ptr<SDL::Texture> &Arcollect::db::artwork::query_texture(void)
{
	static std::unique_ptr<SDL::Texture> null_text;
	// Enforce rating
	if (art_rating > Arcollect::config::current_rating)
		return null_text;
	
	if (!text) {
		// Push me on the loader stack
		Arcollect::db::artwork_loader::pending_main.push_back(query(art_id));
		Arcollect::db::artwork_loader::condition_variable.notify_one();
	}
	return text;
}
int Arcollect::db::artwork::render(const SDL::Rect *dstrect)
{
	std::unique_ptr<SDL::Texture> &text = query_texture();
	if (text)
		return renderer->Copy(text.get(),NULL,dstrect);
	else {
		// Render a placeholder
		renderer->SetDrawColor(0,0,0,192);
		return renderer->FillRect(*dstrect);
	}
}

void Arcollect::db::artwork::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT art_title, art_desc, art_source, art_width, art_height, art_rating FROM artworks WHERE art_artid = ?;",stmt); // TODO Error checking
		stmt->bind(1,art_id);
		if (auto code = stmt->step() == SQLITE_ROW) {
			art_title  = stmt->column_string(0);
			art_desc   = stmt->column_string(1);
			art_source = stmt->column_string(2);
			// Check if picture size is stored
			if ((stmt->column_type(3) == SQLITE_NULL)||(stmt->column_type(4) == SQLITE_NULL)) {
				// Load picture
				SDL::Surface *surf = load_surface();
				text.reset(SDL::Texture::CreateFromSurface(renderer,surf));
				delete surf;
				// Read size
				text->QuerySize(art_size);
				// Set size in the database
				std::unique_ptr<SQLite3::stmt> set_size_stmt;
				database->prepare("UPDATE artworks SET art_width=?, art_height=? WHERE art_artid = ?;",set_size_stmt); // TODO Error checking
				set_size_stmt->bind(1,art_size.x);
				set_size_stmt->bind(2,art_size.y);
				set_size_stmt->bind(3,art_id);
				set_size_stmt->step();
				database->exec("COMMIT;");
			} else {
				// Size is stored
				art_size.x = stmt->column_int64(3);
				art_size.y = stmt->column_int64(4);
			}
			
			art_rating = static_cast<Arcollect::config::Rating>(stmt->column_int64(5));
			data_version = Arcollect::data_version;
		} else {
		}
		
		linked_accounts.clear();
	}
}

const std::vector<std::shared_ptr<Arcollect::db::account>> &Arcollect::db::artwork::get_linked_accounts(const std::string &link)
{
	db_sync();
	auto iterbool = linked_accounts.emplace(link,std::vector<std::shared_ptr<account>>());
	std::vector<std::shared_ptr<account>> &result = iterbool.first->second;
	if (iterbool.second) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT acc_arcoid FROM art_acc_links WHERE art_artid = ? AND artacc_link = ?;",stmt); // TODO Error checking
		;
		stmt->bind(1,art_id);
		stmt->bind(2,link.c_str());
		while (stmt->step() == SQLITE_ROW) {
			result.emplace_back(Arcollect::db::account::query(stmt->column_int64(0)));
			
			data_version = Arcollect::data_version;
		}
	}
	return result;
}

int Arcollect::db::artwork::db_delete(void)
{
	int code;
	std::cerr << "Deleting \"" << art_title << "\" (" << art_id << ")" << std::endl;
	if ((code = database->exec("BEGIN IMMEDIATE;")) != SQLITE_OK) {
		switch (code) {
			case SQLITE_BUSY: {
				std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), \"BEGIN IMMEDIATE;\" failed with SQLITE_BUSY. Another is writing on the database. Abort." << std::endl;
			} return SQLITE_BUSY;
			default: {
				std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), \"BEGIN IMMEDIATE;\" failed: " << database->errmsg() << ". Ignoring..." << std::endl;
			} break;
		}
	}
	std::unique_ptr<SQLite3::stmt> stmt;
	// Delete art_acc_links references
	if (database->prepare("DELETE FROM art_acc_links WHERE art_artid = ?;",stmt) != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to prepare \"DELETE FROM art_acc_links WHERE art_artid = ?;\": " << database->errmsg() << ". Abort." << std::endl;
		return SQLITE_ERROR;
	}
	if (stmt->bind(1,art_id) != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to bind art_artid in \"DELETE FROM art_acc_links WHERE art_artid = ?;\": " << database->errmsg() << ". Abort." << std::endl;
		return SQLITE_ERROR;
	}
	if (stmt->step() != SQLITE_DONE) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to step \"DELETE FROM art_acc_links WHERE art_artid = ?;\": " << database->errmsg() << ". Abort." << std::endl;
		return SQLITE_ERROR;
	}
	
	// Delete art_tag_links references
	if (database->prepare("DELETE FROM art_tag_links WHERE art_artid = ?;",stmt) != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to prepare \"DELETE FROM art_tag_links WHERE art_artid = ?;\": " << database->errmsg() << ". Abort." << std::endl;
		return SQLITE_ERROR;
	}
	if (stmt->bind(1,art_id) != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to bind art_artid in \"DELETE FROM art_tag_links WHERE art_artid = ?;\": " << database->errmsg() << ". Abort." << std::endl;
		return SQLITE_ERROR;
	}
	if (stmt->step() != SQLITE_DONE) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to step \"DELETE FROM art_tag_links WHERE art_artid = ?;\": " << database->errmsg() << ". Abort." << std::endl;
		return SQLITE_ERROR;
	}
	
	// Delete in artworks table
	if (database->prepare("DELETE FROM artworks WHERE art_artid = ?;",stmt) != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to prepare \"DELETE FROM artworks WHERE art_artid = ?;\": " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;"); // TODO Error checkings
		return SQLITE_ERROR;
	}
	if (stmt->bind(1,art_id) != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to bind art_artid in \"DELETE FROM artworks WHERE art_artid = ?;\": " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;"); // TODO Error checkings
		return SQLITE_ERROR;
	}
	if (stmt->step() != SQLITE_DONE) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to step \"DELETE FROM artworks WHERE art_artid = ?;\": " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;"); // TODO Error checkings
		return SQLITE_ERROR;
	}
	// Commit changes
	if (database->exec("COMMIT;") != SQLITE_OK) {
		std::cerr << "Deleting \"" << art_title << "\" (" << art_id << "), failed to commit changes: " << database->errmsg() << ". Rollback." << std::endl;
		database->exec("ROLLBACK;"); // TODO Error checkings
		return SQLITE_ERROR;
	}
	// Erase on disk
	std::filesystem::remove(Arcollect::path::artwork_pool / std::to_string(art_id));
	std::cerr << "Artwork \"" << art_title << "\" (" << art_id << ") has been deleted" << std::endl;
	return 0;
}
