#include "account.hpp"
#include "db.hpp"
#include <arcollect-paths.hpp>
#include <SDL_image.h>
static std::unordered_map<sqlite_int64,std::shared_ptr<Arcollect::db::account>> accounts_pool;

Arcollect::db::account::account(Arcollect::db::account_id arcoid) :
	arcoid(arcoid)
{
	const std::string path = account_avatars_path+std::to_string(arcoid);
	SDL::Surface *surf = (SDL::Surface*)IMG_Load(path.c_str());
	icon.reset(SDL::Texture::CreateFromSurface(renderer,surf));
	delete surf;
}
std::shared_ptr<Arcollect::db::account> &Arcollect::db::account::query(Arcollect::db::account_id arcoid)
{
	std::shared_ptr<Arcollect::db::account> &pointer = accounts_pool.try_emplace(arcoid).first->second;
	if (!pointer)
		pointer = std::shared_ptr<Arcollect::db::account>(new Arcollect::db::account(arcoid));
	return pointer;
}

void Arcollect::db::account::db_sync(void)
{
	if (data_version != Arcollect::data_version) {
		std::unique_ptr<SQLite3::stmt> stmt;
		database->prepare("SELECT acc_name, acc_title, acc_url FROM accounts WHERE acc_arcoid = ?;",stmt); // TODO Error checking
		stmt->bind(1,arcoid);
		if (stmt->step() == SQLITE_ROW) {
			acc_name   = stmt->column_string(0);
			acc_title  = stmt->column_type(1) == SQLITE_NULL ? acc_name : stmt->column_string(1);
			acc_url    = stmt->column_string(2);
			
			data_version = Arcollect::data_version;
		} else {
		}
	}
}
