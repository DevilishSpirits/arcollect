#include "artwork.hpp"
#include "db.hpp"
#include <arcollect-paths.hpp>
#include <SDL_image.h>
static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::artwork>> artworks_pool;

Arcollect::db::artwork::artwork(Arcollect::db::artwork_id art_id) :
	art_id(art_id)
{
	const std::string path = artwork_pool_path+std::to_string(art_id);
	SDL_Surface *surf = IMG_Load(path.c_str());
	text.reset(SDL::Texture::CreateFromSurface(renderer,surf));
	SDL_FreeSurface(surf);
}
std::shared_ptr<Arcollect::db::artwork> &Arcollect::db::artwork::query(Arcollect::db::artwork_id art_id)
{
	std::shared_ptr<Arcollect::db::artwork> &pointer = artworks_pool.try_emplace(art_id).first->second;
	if (!pointer)
		pointer = std::shared_ptr<Arcollect::db::artwork>(new Arcollect::db::artwork(art_id));
	return pointer;
}

void Arcollect::db::artwork::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT art_title, art_desc, art_source FROM artworks WHERE art_artid = ?;",stmt); // TODO Error checking
		stmt->bind(1,art_id);
		if (auto code = stmt->step() == SQLITE_ROW) {
			art_title  = stmt->column_string(0);
			art_desc   = stmt->column_string(1);
			art_source = stmt->column_string(2);
			
			data_version = Arcollect::data_version;
		} else {
		}
	}
}
