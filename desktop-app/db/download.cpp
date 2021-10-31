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
#include "download.hpp"
#include "artwork-loader.hpp"
#include "db.hpp"
#include "../art-reader/image.hpp"
#include "../art-reader/text.hpp"
#include <arcollect-paths.hpp>

std::list<std::reference_wrapper<Arcollect::db::download>> Arcollect::db::download::last_rendered;
static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::download>> downloads_pool;
extern SDL::Renderer *renderer;

Arcollect::db::download::download(sqlite_int64 id, std::string &&source, std::filesystem::path &&path, std::string &&mimetype) :
	artwork_type(artwork_type_from_mime(mimetype)),
	dwn_id      (id),
	dwn_source  (std::move(source)),
	dwn_path    (std::move(path)),
	dwn_mimetype(std::move(mimetype)),
	size{0,0}
{
}

std::shared_ptr<Arcollect::db::download> &Arcollect::db::download::query(sqlite_int64 dwn_id)
{
	static std::shared_ptr<Arcollect::db::download> null_download;
	auto iter = downloads_pool.find(dwn_id);
	if (iter == downloads_pool.end()) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT dwn_source, dwn_path, dwn_mimetype, dwn_width, dwn_height FROM downloads WHERE dwn_id = ?;",stmt); // TODO Error checking
		stmt->bind(1,dwn_id);
		switch (stmt->step()) {
			case SQLITE_ROW: {
				std::string dwn_source;
				if (stmt->column_type(0) == SQLITE_TEXT)
					dwn_source = stmt->column_string(0);
				iter = downloads_pool.emplace(dwn_id,std::make_shared<Arcollect::db::download>(dwn_id,std::move(dwn_source),stmt->column_string(1),stmt->column_string(2))).first;
				// Get art size
				Arcollect::db::download& new_download = *iter->second;
				new_download.size.x = stmt->column_int64(3);
				new_download.size.y = stmt->column_int64(4);
			} break;
			default: {
				// TODO Error report and handling
			} return null_download;
		}
	}
	return iter->second;
}

bool Arcollect::db::download::queue_for_load(void)
{
	if (load_state == LOADED)
		return true;
	Arcollect::db::artwork_loader::pending_main.push_back(query(dwn_id));
	last_render_timestamp = SDL_GetTicks();
	if (load_state == UNLOADED)
		load_state = LOAD_SCHEDULED;
	return false;
}

void Arcollect::db::download::load_stage_one(void)
{
	const std::filesystem::path full_path = Arcollect::path::arco_data_home/dwn_path;
	switch (artwork_type) {
		case ARTWORK_TYPE_UNKNOWN: {
			// TODO Put a message about unknown artwork
		} break;
		case ARTWORK_TYPE_IMAGE: {
			data = std::unique_ptr<SDL::Surface>(art_reader::image(full_path));
		} break;
		case ARTWORK_TYPE_TEXT: {
			data = art_reader::text(full_path,dwn_mimetype);
		} break;
	}
	load_state = LOAD_PENDING_STAGE2;
}
void Arcollect::db::download::load_stage_two(void)
{
	// TODO Check for load failures
	switch (artwork_type) {
		case ARTWORK_TYPE_UNKNOWN: {
		} break;
		case ARTWORK_TYPE_IMAGE: {
			// Generate texture
			SDL::Texture *text = SDL::Texture::CreateFromSurface(renderer,std::get<std::unique_ptr<SDL::Surface>>(data).get());
			if (!text) {
				// TODO Better error handling
				load_state = UNLOADED;
				return;
			}
			data = std::unique_ptr<SDL::Texture>(text);
			// Set size if missing in the DB
			if (!size.x || !size.y) {
				// Read size
				text->QuerySize(size);
				// Set size in the database
				std::unique_ptr<SQLite3::stmt> set_size_stmt;
				database->prepare("UPDATE downloads SET dwn_width=?, dwn_height=? WHERE dwn_id = ?;",set_size_stmt); // TODO Error checking
				set_size_stmt->bind(1,size.x);
				set_size_stmt->bind(2,size.y);
				set_size_stmt->bind(3,dwn_id);
				set_size_stmt->step();
				database->exec("COMMIT;");
			}
			// Increase image memory usage
			Arcollect::db::artwork_loader::image_memory_usage += image_memory();
		} break;
		case ARTWORK_TYPE_TEXT: {
			// Already loaded
		} break;
	}
	// Set last_rendered_iterator
	last_rendered.emplace_front(*this);
	last_rendered_iterator = last_rendered.begin();
	// Update state
	load_state = LOADED;
}
void Arcollect::db::download::unload(void)
{
	switch (artwork_type) {
		case ARTWORK_TYPE_UNKNOWN: {
		} break;
		case ARTWORK_TYPE_IMAGE: {
			// Decrease image memory usage
			Arcollect::db::artwork_loader::image_memory_usage -= image_memory();
		} break;
		case ARTWORK_TYPE_TEXT: {
			// Already loaded
		} break;
	}
	// Unload data
	data.emplace<std::unique_ptr<SDL::Surface>>();
	// Erase myself from last_rendered
	last_rendered.erase(last_rendered_iterator);
	
	load_state = UNLOADED;
}

Arcollect::db::download::ArtworkType Arcollect::db::download::artwork_type_from_mime(const std::string_view& mime)
{
	if ((mime.size() >= 6)
	 &&(mime[0] == 'i')
	 &&(mime[1] == 'm')
	 &&(mime[2] == 'a')
	 &&(mime[3] == 'g')
	 &&(mime[4] == 'e')
	 &&(mime[5] == '/'))
		return ARTWORK_TYPE_IMAGE;
	else if ((mime.size() >= 5)
	 &&(mime[0] == 't')
	 &&(mime[1] == 'e')
	 &&(mime[2] == 'x')
	 &&(mime[3] == 't')
	 &&(mime[4] == '/')) {
		return ARTWORK_TYPE_TEXT;
	} else if (mime == "application/rtf") {
		return ARTWORK_TYPE_TEXT;
	} else return ARTWORK_TYPE_UNKNOWN;
}

std::size_t Arcollect::db::download::image_memory(void)
{
	Uint32 format = SDL_PIXELFORMAT_RGBA32; // Fallback to 32bpp
	auto text = query_data<std::unique_ptr<SDL::Texture>>();
	if (text)
		text->get()->QueryTexture(&format);
	return SDL_BYTESPERPIXEL(format)*sizeof(Uint8)*size.x*size.y; // Assume 8-bits RGBA
}

void Arcollect::db::download::nuke_image_cache(void)
{
	// Shutdown threads
	Arcollect::db::artwork_loader::shutdown_sync();
	// Unload images
	auto iter = Arcollect::db::download::last_rendered.begin();
	while (iter != Arcollect::db::download::last_rendered.end()) {
		Arcollect::db::download& download = *iter;
		++iter;
		if (download.artwork_type == ARTWORK_TYPE_IMAGE)
			download.unload();
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

bool Arcollect::db::download::delete_cache(sqlite3_int64 dwn_id, Transaction& transaction)
{
	bool to_nuke = transaction.delete_cache(dwn_id);
	if (to_nuke) {
		auto iter = downloads_pool.find(dwn_id);
		if (iter == downloads_pool.end()) {
			downloads_pool.erase(iter);
		}
	}
	return to_nuke;
}
