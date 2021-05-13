#pragma once
#include <sqlite3.hpp>
#include "../sdl2-hpp/SDL.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
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
				SDL::Point  art_size;
				std::unordered_map<std::string,std::vector<std::shared_ptr<account>>> linked_accounts;
			public:
				/** Load the image in a SDL::Surface
				 *
				 * This is a convenience function.
				 */
				SDL::Surface *load_surface(void);
				// Private but public in int main(void)
				std::unique_ptr<SDL::Texture> text;
				// Delete copy constructor
				artwork(const artwork&) = delete;
				artwork& operator=(artwork&) = delete;
				inline void QuerySize(SDL::Point &size) {
					db_sync();
					size = art_size;
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
				
				/** Query an artwork
				 * \param art_id The artwork identifier
				 * \return The artwork wrapped in a std::shared_ptr
				 * This function create or return a cached version of the #Arcollect::db::artwork.
				 */
				static std::shared_ptr<artwork> &query(Arcollect::db::artwork_id art_id);
		};
	}
}
