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

// Provide a dummy semaphore if compiler doesn't support it.
#define counting_semaphore arcollect_counting_semaphore
#if __has_include(<semaphore>)
#include <semaphore>
#if __cpp_lib_semaphore>=201907
	//#undef counting_semaphore // Compiler provide semaphore support
#endif
#endif
#ifdef counting_semaphore
#include <atomic>
namespace std {
	// Mock a semaphore
	template<std::ptrdiff_t max_value = 65536>
	struct counting_semaphore {
		std::atomic<std::ptrdiff_t> atomic_value;
		constexpr explicit counting_semaphore(std::ptrdiff_t desired) : atomic_value(desired) {}
		template<class Rep, class Period> bool try_acquire_for( const std::chrono::duration<Rep, Period>& rel_time ) {
			std::ptrdiff_t old_value = atomic_value--;
			if (old_value <= 0) {
				// Failed to acquire, restore then return false;
				atomic_value++;
				return false;
			} else return true;
		}
		void release( std::ptrdiff_t update = 1 ) {
			atomic_value += update;
		}
	};
}
#endif

std::list<std::reference_wrapper<Arcollect::db::download>> Arcollect::db::download::last_rendered;
static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::download>> downloads_pool;

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
	// Refresh last_render timestamps
	last_render_timestamp = SDL_GetTicks();
	// Bring to the front of  last_rendered list
	if (load_state == LOADED) {
		last_rendered.splice(last_rendered.begin(),last_rendered,last_rendered_iterator);
		return true;
	}
	// Schedule loading
	Arcollect::db::artwork_loader::pending_main.push_back(query(dwn_id));
	if (load_state == UNLOADED)
		load_state = LOAD_SCHEDULED;
	return false;
}

void Arcollect::db::download::queue_full_image_for_load(void)
{
	query_image(Arcollect::art_reader::nothumbnail_size);
}

struct SurfacePixelBordersIterate {
	SDL::Surface& surface;
	struct iterator {
		SDL::Surface& surf;
		SDL::Point position;
		bool start;
		SDL::Color &operator*(void) const {
			return *reinterpret_cast<SDL::Color*>(&(static_cast<Uint8*>(surf.pixels)[surf.pitch*position.y+position.x*surf.format->BytesPerPixel]));
		}
		constexpr iterator& operator++(void) {
			start = false;
			// Top left-to-right scan
			if (!position.y)
				if (position.x == surf.w-1)
					position.y++;
				else position.x++;
			// Right top-to-bottom scan
			else if (position.x == surf.w-1)
				if (position.y == surf.h-1)
					position.x--;
				else position.y++;
			// Top right-to-keft scan
			else if (position.x)
				position.x--;
			// Left bottom-to-top scan
			else position.y--;
			return *this;
		}
		constexpr bool operator!=(const iterator& other) const {
			return (position != other.position)||(start != other.start);
		}
		iterator(SDL::Surface& surface, bool start) : surf(surface), position{0,0}, start(start) {}
	};
	iterator begin(void) {
		return iterator(surface,true);
	}
	iterator end(void) {
		return iterator(surface,false);
	}
	SurfacePixelBordersIterate(SDL::Surface& surface) : surface(surface) {}
};

static bool pixel_art_scan(SDL::Surface& surf)
{
	if ((surf.w > 64)&&(surf.h > 64)) {
		/* We perform horizontal and vertical scans of the image and break if we
		 * spot that not two pixels are the same.
		 *
		 * This is trivial and not 100% effective. Further research is welcome.
		 */
		static constexpr auto nx_scans = 64;
		static constexpr auto ny_scans = 64;
		
		Uint8 *pixels = static_cast<Uint8*>(surf.pixels);
		Uint32 color_mask = surf.format->Rmask|surf.format->Gmask|surf.format->Bmask|surf.format->Amask;
		color_mask &= 0xF0F0F0F0; // Strip low order bits to counter lossy compression
		// Horizontal scans
		std::ptrdiff_t xscan_stride = surf.pitch*((surf.h/nx_scans)-1); // Note that the cursor is one-line below the start so we remove 1
		std::ptrdiff_t xscan_step = surf.format->BytesPerPixel;
		Uint8 *xscan_end = pixels+(xscan_stride+xscan_step*surf.w)*nx_scans;
		for (Uint8 *pix = pixels; pix != xscan_end; pix += xscan_stride) {
			// Init
			Uint8 *line_end = pix + xscan_step*surf.w;
			// Handle the first pixel
			Uint32 last_color = *reinterpret_cast<Uint32*>(pix) & color_mask;
			pix += xscan_step;
			bool was_same = true; // Skip possible 1-pixel border
			for (; pix != line_end; pix += xscan_step) {
				Uint32 curr_color = *reinterpret_cast<Uint32*>(pix) & color_mask;
				if (last_color != curr_color) {
					if (!was_same)
						return false;
					last_color = curr_color;
					was_same = false;
				} else was_same = true;
			}
		}
		// Vertical scans
		std::ptrdiff_t yscan_stride = surf.format->BytesPerPixel*(surf.w/ny_scans) - surf.h*surf.pitch; // Also reset the cursor to the topmost pixel
		std::ptrdiff_t yscan_step = surf.pitch;
		Uint8 *yscan_end = pixels+(yscan_stride+yscan_step*surf.h)*ny_scans;
		for (Uint8 *pix = pixels; pix != yscan_end; pix += yscan_stride) {
			// Init
			Uint8 *line_end = pix + yscan_step*surf.h;
			// Handle the first pixel
			Uint32 last_color = *reinterpret_cast<Uint32*>(pix) & color_mask;
			pix += yscan_step;
			bool was_same = true; // Skip possible 1-pixel border
			for (; pix != line_end; pix += yscan_step) {
				Uint32 curr_color = *reinterpret_cast<Uint32*>(pix) & color_mask;
				if (last_color != curr_color) {
					if (!was_same)
						return false;
					last_color = curr_color;
					was_same = false;
				} else was_same = true;
			}
		}
		// We got there, pixel art!
		return true;
	} else return true; // Tiny images render best as pixel art since they'd be excessively blurred otherwhise
}

// Avoid excessive memory bursts
static std::counting_semaphore images_loadlimit(std::min<unsigned int>(std::thread::hardware_concurrency(),2));

void Arcollect::db::download::load_stage_one(void)
{
	const std::filesystem::path full_path = Arcollect::path::arco_data_home/dwn_path;
	switch (artwork_type) {
		case ARTWORK_TYPE_UNKNOWN: {
			// TODO Put a message about unknown artwork
		} break;
		case ARTWORK_TYPE_IMAGE: {
			// Lock the semaphore
			while (!images_loadlimit.try_acquire_for(std::chrono::seconds(1)))
				if (load_state == LOADING_STAGE1)
					break;
			// Don't load thumbnail if we ignore the artwork size
			if (!requested_size.x || !requested_size.y || !size.x || !size.y)
				requested_size = Arcollect::art_reader::nothumbnail_size;
			data = std::unique_ptr<SDL::Surface>(art_reader::image(full_path,requested_size));
			if (!std::get<std::unique_ptr<SDL::Surface>>(data))
				break;
			SDL::Surface &surf = *std::get<std::unique_ptr<SDL::Surface>>(data);
			if ((surf.w > 2)&&(surf.h > 2)) {
				/* Try to auto-detect the best background color
				 *
				 * Colors on the border are averaged and a simplified deviation computed
				 * to guess if there is a flat border background color to expand on the
				 * rest of the image.
				 * The amount of pixels exceding a deviation threshold is also counted,
				 * if the amount of deviant pixels exceed a threshold (0,4%), the image
				 * is considered to have no border too.
				 *
				 * This allow some noise (JPEG, ...) while staying fairly sensitive to
				 * border irregularities. Some botder gradients still pass the test,
				 * actually many fradients are visually improved but 
				 * 
				 * Most scans does not pass the test, but the limit would be noticable
				 * and not beautiful. Before thinking that the system is not working,
				 * look closely at the image if there is irregularities.
				 *
				 * Best results are on almost-black background where this effect is not
				 * perceived but lack would be a lot.
				 */
				int64_t red = 0;
				int64_t green = 0;
				int64_t blue = 0;
				// Accumulate pixels on the border
				for (const SDL::Color &color: SurfacePixelBordersIterate(surf)) {
					red += color.r;
					green += color.g;
					blue += color.b;
				}
				// Compute the average
				// Note that we are rounding to nearest, not to zero
				const auto pix_count = 2*(surf.w+surf.h);
				red <<= 1;
				green <<= 1;
				blue <<= 1;
				red /= pix_count;
				green /= pix_count;
				blue /= pix_count;
				red += 1;
				green += 1;
				blue += 1;
				red >>= 1;
				green >>= 1;
				blue >>= 1;
				// Compute a simplified deviation
				int64_t deviation = 0;
				int64_t pixels_out_deviation = 0;
				for (const SDL::Color &color: SurfacePixelBordersIterate(surf)) {
					int64_t this_deviation = abs(red - static_cast<int>(color.r)) + abs(green - static_cast<int>(color.g)) + abs(blue - static_cast<int>(color.b));
					if (this_deviation > 48)
						pixels_out_deviation++;
					deviation += this_deviation;
				}
				deviation /= pix_count;
				if ((deviation < 48)&&(pixels_out_deviation <= pix_count/256)) {
					background_color.r = red;
					background_color.g = green;
					background_color.b = blue;
					background_color.a = 255;
				}
			}
			is_pixel_art = pixel_art_scan(surf);
		} break;
		case ARTWORK_TYPE_TEXT: {
			data = art_reader::text(full_path,dwn_mimetype);
		} break;
	}
	load_state = LOAD_PENDING_STAGE2;
}
void Arcollect::db::download::load_stage_two(SDL::Renderer &renderer)
{
	// TODO Check for load failures
	switch (artwork_type) {
		case ARTWORK_TYPE_UNKNOWN: {
		} break;
		case ARTWORK_TYPE_IMAGE: {
			// Generate texture
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,is_pixel_art ? "nearest" : "best");
			SDL::Texture *text = SDL::Texture::CreateFromSurface(&renderer,std::get<std::unique_ptr<SDL::Surface>>(data).get());
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"best");
			if (!text) {
				// TODO Better error handling
				load_state = UNLOADED;
				return;
			}
			data = std::unique_ptr<SDL::Texture>(text);
			text->QuerySize(loaded_size);
			// Set size if missing in the DB
			if (!size.x || !size.y) {
				// Read size
				size = loaded_size;
				// Set size in the database
				std::unique_ptr<SQLite3::stmt> set_size_stmt;
				database->prepare("UPDATE downloads SET dwn_width=?, dwn_height=? WHERE dwn_id = ?;",set_size_stmt); // TODO Error checking
				set_size_stmt->bind(1,size.x);
				set_size_stmt->bind(2,size.y);
				set_size_stmt->bind(3,dwn_id);
				set_size_stmt->step();
				database->exec("COMMIT;");
			}
			// Erase transient thumbnail
			if (transient_thumbnail) {
				SDL::Point thumbnail_size;
				transient_thumbnail->QuerySize(thumbnail_size);
				Arcollect::db::artwork_loader::image_memory_usage -= 4*sizeof(Uint8)*thumbnail_size.x*thumbnail_size.y;
				transient_thumbnail.reset();
			}
			// Increase image memory usage
			Arcollect::db::artwork_loader::image_memory_usage += 4*sizeof(Uint8)*loaded_size.x*loaded_size.y;
			images_loadlimit.release();
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
			// Reset thumbnail stuff
			requested_size.x = requested_size.y = 0;
			if (transient_thumbnail) {
				SDL::Point thumbnail_size;
				transient_thumbnail->QuerySize(thumbnail_size);
				Arcollect::db::artwork_loader::image_memory_usage -= 4*sizeof(Uint8)*thumbnail_size.x*thumbnail_size.y;
				transient_thumbnail.reset();
			}
			// Decrease image memory usage
			Arcollect::db::artwork_loader::image_memory_usage -= 4*sizeof(Uint8)*loaded_size.x*loaded_size.y;
			if (load_state == LOAD_PENDING_STAGE2)
				images_loadlimit.release();
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
	if (mime.starts_with("image/"))
		return ARTWORK_TYPE_IMAGE;
	else if (mime.starts_with("text/")) {
		return ARTWORK_TYPE_TEXT;
	} else if (mime == "application/rtf") {
		return ARTWORK_TYPE_TEXT;
	} else return ARTWORK_TYPE_UNKNOWN;
}

std::size_t Arcollect::db::download::image_memory(void)
{
	return 4*sizeof(Uint8)*size.x*size.y; // Assume 8-bits RGBA
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
