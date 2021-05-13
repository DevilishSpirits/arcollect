#include "artwork.hpp"
#include "artwork-loader.hpp"
#include "account.hpp"
#include "db.hpp"
#include <arcollect-paths.hpp>
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
int Arcollect::db::artwork::render(const SDL::Rect *dstrect)
{
	if (text)
		return renderer->Copy(text.get(),NULL,dstrect);
	else {
		// TODO Render a placeholder
		Arcollect::db::artwork_loader::pending_main.push_back(query(art_id));
		Arcollect::db::artwork_loader::condition_variable.notify_one();
		return 2;
	}
}

void Arcollect::db::artwork::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT art_title, art_desc, art_source, art_width, art_height FROM artworks WHERE art_artid = ?;",stmt); // TODO Error checking
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
