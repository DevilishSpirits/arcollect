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
#pragma once
#include <sqlite3.hpp>
#include "../gui/font.hpp"
#include "../config.hpp"
#include <arcollect-db-downloads.hpp>
#include <cstddef>
#include <filesystem>
#include <list>
#include <memory>
#include <variant>

namespace SDL {
	struct Renderer;
}
namespace Arcollect {
	namespace db {
		/** Download
		 *
		 * This class hold data about a download. It's used trough a
		 * std::shared_ptr to maintain one instance per download.
		 *
		 * It is instanciated on-demand trough Arcollect::db::download::query().
		 */
		class download {
			public:
				/** Convenience alias
				 */
				using Transaction = Arcollect::db::downloads::Transaction;
				/** Loading stages of a #download
				 */
				volatile enum LoadState {
					/** Not loaded, nor queued
					 *
					 * Download is not loaded nor scheduled to load in any queue
					 */
					UNLOADED,
					/** Download scheduled to load
					 *
					 * Download has been requested but is not yet into #artwork_loader
					 * threads side. It will be pushed onto in the end of the main-loop.
					 */
					LOAD_SCHEDULED,
					/** Download waiting for stage1 load
					 *
					 * Download is into #artwork_loader threads queue and will be loaded.
					 */
					LOAD_PENDING_STAGE1,
					/** Stage 1 of download loading is ongoing
					 *
					 * Download is processed into one #artwork_loader thread.
					 * This state is necessary as multiple #artwork_loader might pick the
					 * same download in the same time
					 */
					LOADING_STAGE1,
					/** Stage 1 of loading done, waiting for stage 2
					 *
					 * Download has been loaded and await the main loop to handle it.
					 * For images an #SDL::Surface but no #SDL::Texture yet (the later
					 * must be done in the main thread).
					 */
					LOAD_PENDING_STAGE2,
					/** Artwork is ready to use
					 *
					 * Artwork has a #SDL::Texture
					 */
					LOADED,
				} load_state = UNLOADED;
				/** Artwork type
				 *
				 * It change the way artworks are shown.
				 */
				const enum ArtworkType {
					ARTWORK_TYPE_UNKNOWN,
					ARTWORK_TYPE_IMAGE,
					ARTWORK_TYPE_TEXT,
				} artwork_type;
				/** Detech the #ArtworkType from a MIME type
				 * \param mime The MIME type to probe
				 * \return The deducted artwork_type
				 */
				static ArtworkType artwork_type_from_mime(const std::string_view& mime);
			private:
				// FIXME friend artwork_loader; // For queued_for_load
				
				std::variant<
					// Transient formats
					std::unique_ptr<SDL::Surface>,
					// Loaded artworks
					std::unique_ptr<SDL::Texture>,
					gui::font::Elements
				> data;
				std::list<std::reference_wrapper<download>>::iterator last_rendered_iterator;
				
				/** NSFW material taint level
				 *
				 * Rating is not stored within the download but in artworks data, this
				 * value allow to reduce the risk of displaying NSFW is not wanted.
				 *
				 * This is an **additional** security, attempting to query NSFW material
				 * when filter is turned on IS a bug! But given the potentally dramatic
				 * consequences of this kind of bug, Arcollect has always put an extra
				 * security barrier.
				 */
				Arcollect::config::Rating rating_taint_level = Arcollect::config::RATING_NONE;
				/** Transient thumbnail
				 *
				 * Used to display the existing thumbnail while loading a bigger image.
				 */
				std::unique_ptr<SDL::Texture> transient_thumbnail;
				/** The loaded thumbnail/image size
				 */
				SDL::Point loaded_size;
				/** The requested thumbnail size
				 */
				SDL::Point requested_size;
			public:
				download(sqlite_int64 id, std::string &&source, std::filesystem::path &&path, std::string &&mimetype);
				/** Query download for loading
				 * \return true if the artwork is already loaded
				 *
				 * The download is pushed onto the artwork loader stack not loaded or
				 * loading.
				 */
				bool queue_for_load(void);
				/** Query the full resolution image download for loading
				 *
				 * Wrapper for queue_for_load() that set the requested_size to the max.
				 */
				void queue_full_image_for_load(void);
				
				/** Load (thread-safe part)
				 *
				 * Load the download in memory. This is the thread-safe part, you must
				 * call load_stage_two() after in the main thread.
				 */
				void load_stage_one(void);
				
				/** Load (non thread-safe part)
				 * \param renderer A reference to the current renderer
				 *
				 * Finish to load the artwork.
				 * \warning This must be called in LOAD_PENDING_SYNC state!
				 */
				void load_stage_two(SDL::Renderer& renderer);
				
				/** Unload
				 *
				 * Unload the download from memory.
				 */
				void unload(void);
				
				/** Query the download
				 * \return An optional reference to the artwork
				 * 
				 * queue_for_load() if the artwork isn't loaded yet.
				 */
				template <typename T>
				std::optional<std::reference_wrapper<T>> query_data(void) {
					if ((rating_taint_level <= Arcollect::config::current_rating)&&queue_for_load()) {
						last_rendered.splice(last_rendered.begin(),last_rendered,last_rendered_iterator);
						last_render_timestamp = SDL_GetTicks();
						return std::get<T>(data);
					} else return std::nullopt;
				}
				/** Query the download image
				 * \param query_size to display
				 * \return A reference to the texture, may be a thumbnail
				 * 
				 * queue_for_load() if the artwork isn't loaded yet.
				 */
				std::unique_ptr<SDL::Texture> &query_image(SDL::Point query_size) {
					requested_size.x = std::max(requested_size.x,query_size.x);
					requested_size.y = std::max(requested_size.y,query_size.y);
					if ((artwork_type == ARTWORK_TYPE_IMAGE)&& queue_for_load()) {
						std::unique_ptr<SDL::Texture> &res = std::get<std::unique_ptr<SDL::Texture>>(data);
						// Check if we loaded a thumbnail and it is too small
						if (((loaded_size.x != size.x)||(loaded_size.y != size.y))&&((loaded_size.x < query_size.x)||(loaded_size.y < query_size.y))) {
							std::unique_ptr<SDL::Texture> thumbnail = std::move(res);
							unload();
							requested_size = query_size;
							queue_for_load();
							return transient_thumbnail = std::move(thumbnail);
						} else return res;
					} else return transient_thumbnail;
				}
				bool QuerySize(SDL::Point &art_size) {
					if (size.x && size.y) {
						art_size = size;
						return true;
					} else {
						queue_for_load();
						return false;
					}
				}
				
				// Delete copy constructor
				download(const download&) = delete;
				download& operator=(download&) = delete;
				/** The database download id
				 */
				const sqlite_int64          dwn_id;
				const std::string           dwn_source;
				const std::filesystem::path dwn_path;
				const std::string           dwn_mimetype;
				SDL::Point size;
				SDL::Color background_color{0,0,0,0};
				
				/** Taint the download as NSFW material
				 *
				 * Rating is not stored within the download but in artworks data, this
				 * value allow to reduce the risk of displaying NSFW is not wanted.
				 *
				 * This is an **additional** security, attempting to query NSFW material
				 * when filter is turned on IS a bug! But given the potentally dramatic
				 * consequences of this kind of bug, Arcollect has always put an extra
				 * security barrier.
				 */
				void taint(Arcollect::config::Rating rating) {
					rating_taint_level = std::max(rating_taint_level,rating);
				}
				/** Reset the NSFW taint the download
				 *
				 * See taint(), this variant can lower the taint level.
				 * \warning **DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING!!!**
				 */
				void reset_taint(Arcollect::config::Rating rating) {
					rating_taint_level = rating;
				}
				
				/** Estimate VRAM usage of this artwork
				 *
				 * It require the download to be an image and loaded to work correctly. 
				 * But it's fine as an estimation (not for accounting as for
				 * #Arcollect::db::artwork_loader::image_memory_usage).
				 */
				std::size_t image_memory(void);
				
				/** Query a download
				 * \param art_id The download identifier
				 * \return The download wrapped in a std::shared_ptr
				 * This function create or return a cached version of the #Arcollect::db::download.
				 */
				static std::shared_ptr<download> &query(sqlite_int64 dwn_id);
				
				/** Last render list
				 *
				 * This list is used to tack which downloads has been rendered most
				 * recently and to only unload downloads which have not been rendered
				 * recently.
				 *
				 * The front contain the most recently rendered download
				 */
				static std::list<std::reference_wrapper<download>> last_rendered;
				/** Timestamp of last render attempt
				 *
				 * It's used to don't load artworks which have not been requested to
				 * load since a while but are still queued for render.
				 */
				Uint32 last_render_timestamp = 0;
				
				/** Nuke a download
				 * \param art_id The download identifier
				 *
				 * This function remove a download from the cache of `std::shared_ptr`.
				 * \warning It does not remove the download from the database!
				 */
				static void nuke(sqlite_int64 dwn_id);
				/** Clear image cache
				 *
				 * This function destroy everything in image caches.
				 * It is used when changing screens ICC profile.
				 */
				static void nuke_image_cache(void);
				
				/** Compare download
				 *
				 * For internal use.
				 * You should never have to compare downloads as they are always unique.
				 */
				constexpr bool operator==(const download& other) const {
					return dwn_id == other.dwn_id;
				}
				/** Attempt to delete a download
				 * \param dwn_id to remove
				 * \parem transaction to use
				 * \return `true` if the downloadhas already been nuke() and will be 
				 *         deleted upon transaction commit.
				 *
				 * It may delete it or not if used by another key.
				 * \warning It relies on SQLite foreign keys enforcement.
				 */
				static bool delete_cache(sqlite3_int64 id, Transaction& transaction);
		};
	}
}
namespace std {
	template <>
	struct hash<Arcollect::db::download> {
		std::size_t operator()(const Arcollect::db::download& download) const {
			return std::hash<sqlite_int64>()(download.dwn_id);
		}
	};
}
