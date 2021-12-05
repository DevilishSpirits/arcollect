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
#include "../art-reader/image.hpp"
#include "../art-reader/text.hpp"
#include <arcollect-paths.hpp>
#include <arcollect-sqls.hpp>
#include <iostream>
#include <fstream>

static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::artwork>> artworks_pool;
std::list<std::reference_wrapper<Arcollect::db::artwork>> Arcollect::db::artwork::last_rendered;
extern SDL::Renderer *renderer;

Arcollect::db::artwork::artwork(Arcollect::db::artwork_id art_id) :
	data_version(-2),
	art_id(art_id)
{
	db_sync();
}
std::shared_ptr<Arcollect::db::artwork> &Arcollect::db::artwork::query(Arcollect::db::artwork_id art_id)
{
	std::shared_ptr<Arcollect::db::artwork> &pointer = artworks_pool.try_emplace(art_id).first->second;
	if (!pointer)
		pointer = std::shared_ptr<Arcollect::db::artwork>(new Arcollect::db::artwork(art_id));
	return pointer;
}

void Arcollect::db::artwork::queue_for_load(void)
{
	Arcollect::db::artwork_loader::pending_main.push_back(query(art_id));
	last_render_timestamp = SDL_GetTicks();
	if (load_state == UNLOADED)
		load_state = LOAD_SCHEDULED;
}
SDL::Surface *Arcollect::db::artwork::load_surface(void) const
{
	return art_reader::image(image_path());
}
void Arcollect::db::artwork::texture_loaded(std::unique_ptr<SDL::Texture> &texture)
{
	// Set texture
	text = std::move(texture);
	// Set size if missing in the DB
	if (!art_size.x || !art_size.y) {
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
	}
	// Set last_rendered_iterator
	last_rendered.emplace_front(*this);
	last_rendered_iterator = last_rendered.begin();
	// Increase image memory usage
	Arcollect::db::artwork_loader::image_memory_usage += image_memory();
	
	load_state = LOADED;
}
void Arcollect::db::artwork::texture_unload(void)
{
	// Free texture
	text.reset();
	// Erase myself from last_rendered
	last_rendered.erase(last_rendered_iterator);
	// Decrease image memory usage
	Arcollect::db::artwork_loader::image_memory_usage -= image_memory();
	
	load_state = UNLOADED;
}
std::unique_ptr<SDL::Texture> &Arcollect::db::artwork::query_texture(void)
{
	static std::unique_ptr<SDL::Texture> null_text;
	// Enforce rating
	// Note: Returning null_text IS a bug. This is a safety to avoid accidents.
	if (art_rating > Arcollect::config::current_rating)
		return null_text;
	
	if (!text)
		queue_for_load();
	last_render_timestamp = SDL_GetTicks();
	
	return text;
}

const Arcollect::gui::font::Elements &Arcollect::db::artwork::query_font_elements(void)
{
	if (artwork_text_elements.empty()) {
		// Load the artwork
		const std::filesystem::path path = Arcollect::path::artwork_pool / std::to_string(art_id);
		artwork_text_elements = art_reader::text(path,mimetype());
	}
	return artwork_text_elements;
}

int Arcollect::db::artwork::render(const SDL::Rect *dstrect)
{
	std::unique_ptr<SDL::Texture> &text = query_texture();
	if (text) {
		// Bump me in last_rendered list
		last_rendered.splice(last_rendered.begin(),last_rendered,last_rendered_iterator);
		// Render
		return renderer->Copy(text.get(),NULL,dstrect);
	}else {
		// Render a placeholder
		renderer->SetDrawColor(0,0,0,192);
		renderer->FillRect(*dstrect);
		if (dstrect) {
			SDL::Rect placeholder_rect{dstrect->x,dstrect->y,dstrect->h/4,dstrect->h/8};
			if (placeholder_rect.w > dstrect->w*3/4) {
				placeholder_rect.w = dstrect->w*3/4;
			}
			placeholder_rect.x += (dstrect->w-placeholder_rect.w)/2;
			placeholder_rect.y += (dstrect->h-placeholder_rect.h)/2;
			int bar_count;
			const int max_bar_count = 3;
			switch (load_state) {
				case UNLOADED: {
					bar_count = 0;
				} break;
				case LOAD_SCHEDULED:
				case LOAD_PENDING: {
					bar_count = 1;
				} break;
				case READING_PIXELS: {
					bar_count = 2;
				} break;
				case SURFACE_AVAILABLE:
				case LOADED: {
					bar_count = 3;
				} break;
			}
			renderer->SetDrawColor(255,255,255,128);
			renderer->DrawRect(placeholder_rect);
			// Draw inside bars
			auto border = placeholder_rect.w/8;
			placeholder_rect.x += border;
			placeholder_rect.y += border;
			placeholder_rect.h -= border*2;
			placeholder_rect.w -= border*(1+max_bar_count);
			placeholder_rect.w /= max_bar_count;
			const SDL::Color bar_colors[] = {
				{0x000000FF},
				{0x808080FF},
				{0x00ff00FF},
				{0x00ff00FF},
			};
			for (auto i = bar_count; i; i--) {
				const SDL::Color &color = bar_colors[bar_count];
				renderer->SetDrawColor((color.r+(rand()%256))/2,(color.g+(rand()%256))/2,(color.b+(rand()%256))/2,64);
				renderer->FillRect(placeholder_rect);
				renderer->SetDrawColor(color.r,color.g,color.b,128);
				renderer->DrawRect(placeholder_rect);
				placeholder_rect.x += placeholder_rect.w+border;
			}
		}
		return 0;
	}
}

static std::string column_string_default(std::unique_ptr<SQLite3::stmt> &stmt, int col)
{
	if (stmt->column_type(col) == SQLITE_NULL)
		return "";
	else return stmt->column_string(col);
}

void Arcollect::db::artwork::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT art_title, art_desc, art_source, art_width, art_height, art_rating, art_mimetype FROM artworks WHERE art_artid = ?;",stmt); // TODO Error checking
		stmt->bind(1,art_id);
		if (stmt->step() == SQLITE_ROW) {
			art_title  = column_string_default(stmt,0);
			art_desc   = column_string_default(stmt,1);
			art_source = stmt->column_string(2);
			// Load picture size if unknow
			if (!art_size.x || !art_size.y) {
				art_size.x = stmt->column_int64(3);
				art_size.y = stmt->column_int64(4);
			}
			
			art_rating = static_cast<Arcollect::config::Rating>(stmt->column_int64(5));
			
			// Detect artwork type
			art_mimetype = stmt->column_string(6);
			if ((art_mimetype.size() >= 6)
			 &&(art_mimetype[0] == 'i')
			 &&(art_mimetype[1] == 'm')
			 &&(art_mimetype[2] == 'a')
			 &&(art_mimetype[3] == 'g')
			 &&(art_mimetype[4] == 'e')
			 &&(art_mimetype[5] == '/'))
				artwork_type = ARTWORK_TYPE_IMAGE;
			else if ((art_mimetype.size() >= 5)
			 &&(art_mimetype[0] == 't')
			 &&(art_mimetype[1] == 'e')
			 &&(art_mimetype[2] == 'x')
			 &&(art_mimetype[3] == 't')
			 &&(art_mimetype[4] == '/')) {
				artwork_type = ARTWORK_TYPE_TEXT;
			} else if (art_mimetype == "application/rtf") {
				artwork_type = ARTWORK_TYPE_TEXT;
			} else artwork_type = ARTWORK_TYPE_UNKNOWN;
			
			data_version = Arcollect::data_version;
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

std::size_t Arcollect::db::artwork::image_memory(void)
{
	Uint32 format = SDL_PIXELFORMAT_RGBA32; // Fallback to 32bpp
	if (text)
		text->QueryTexture(&format);
	return SDL_BYTESPERPIXEL(format)*sizeof(Uint8)*art_size.x*art_size.y; // Assume 8-bits RGBA
}

#ifdef ARTWORK_HAS_OPEN_URL
void Arcollect::db::artwork::open_url(void)
{
	SDL_OpenURL(source().c_str());
}
#endif

void Arcollect::db::artwork::nuke_image_cache(void)
{
	// Shutdown threads
	Arcollect::db::artwork_loader::shutdown_sync();
	// Unload images
	for (auto &art: artworks_pool) {
		Arcollect::db::artwork& artwork = *(art.second);
		artwork.text.reset();
		artwork.load_state = Arcollect::db::artwork::UNLOADED;
	}
	// Clear various buffers
	// Note: Loader threads are not running so we don't need to take the mutex
	last_rendered.clear();
	Arcollect::db::artwork_loader::pending_main.clear();
	Arcollect::db::artwork_loader::pending_thread_first.clear();
	Arcollect::db::artwork_loader::pending_thread_second.clear();
	Arcollect::db::artwork_loader::done.clear();
	Arcollect::db::artwork_loader::image_memory_usage = 0;
	// Restart threads
	Arcollect::db::artwork_loader::start();
}
