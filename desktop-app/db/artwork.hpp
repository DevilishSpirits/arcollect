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
#include "../sdl2-hpp/SDL.hpp"
#include "../config.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

#if WITH_XDG
	#define ARTWORK_HAS_OPEN_URL
#endif

extern SDL::Renderer *renderer;
int main(void);
namespace Arcollect {
	namespace db {
		typedef sqlite_int64 artwork_id;
		class artwork;
		class account;
		/** Artwork data class holder
		 *
		 * This class hold data about an artwork. It's used trough a std::shared_ptr
		 * to maintain one instance per artwork.
		 *
		 * It is instanciated on-demand trough Arcollect::db::artwork::query().
		 */
		class artwork {
			private:
				artwork(Arcollect::db::artwork_id art_id);
				// Cached DB infos
				sqlite_int64 data_version;
				void db_sync(void);
				std::string art_title;
				std::string art_desc;
				std::string art_source;
				SDL::Point  art_size{0,0};
				Arcollect::config::Rating art_rating;
				std::unordered_map<std::string,std::vector<std::shared_ptr<account>>> linked_accounts;
				std::unique_ptr<SDL::Texture> text;
			public:
				/** Query artwork loading
				 *
				 * The artwork is pushed onto the artwork loader stack.
				 */
				void queue_for_load(void);
				/** Load the image in a SDL::Surface
				 * \return A surface containing the artwork
				 * 
				 * This is a convenience blocking function that is called in a dedicated
				 * thread.
				 */
				SDL::Surface *load_surface(void) const;
				/** Set the loaded texture
				 * \param texture The loaded texture. This function steal it.
				 *
				 * This function is called by the main thread after load_surface() end
				 * in the loader thread.
				 */
				void texture_loaded(std::unique_ptr<SDL::Texture> &texture);
				/** Query the texture
				 * \return The texture or NULL if it is not loaded right-now.
				 * 
				 * This function queue the artwork for loading if it is not loaded yet.
				 */
				std::unique_ptr<SDL::Texture> &query_texture(void);
				// Delete copy constructor
				artwork(const artwork&) = delete;
				artwork& operator=(artwork&) = delete;
				inline bool QuerySize(SDL::Point &size) {
					db_sync();
					size = art_size;
					
					// Queue for load if size is unknow
					bool loaded = art_size.x && art_size.y;
					if (!loaded)
						queue_for_load();
					
					return loaded;
				}
				/** The database artwork id
				 */
				const Arcollect::db::artwork_id art_id;
				
				int render(const SDL::Rect *dstrect);
				
				inline const std::string &title(void) {
					db_sync();
					return art_title;
				}
				inline const std::string &desc(void) {
					db_sync();
					return art_desc;
				}
				inline const std::string &source(void) {
					db_sync();
					return art_source;
				}
				
				const std::vector<std::shared_ptr<account>> &get_linked_accounts(const std::string &link);
				
				#ifdef ARTWORK_HAS_OPEN_URL
				/** Open the artwork URL
				 */
				void open_url(void);
				#endif
				
				/** Delete an artwork
				 *
				 * This is not an innocent function !
				 */
				int db_delete(void);
				
				/** Query an artwork
				 * \param art_id The artwork identifier
				 * \return The artwork wrapped in a std::shared_ptr
				 * This function create or return a cached version of the #Arcollect::db::artwork.
				 */
				static std::shared_ptr<artwork> &query(Arcollect::db::artwork_id art_id);
		};
	}
}
