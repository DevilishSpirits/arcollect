#pragma once
#include <sqlite3.hpp>
#include "../sdl2-hpp/SDL.hpp"
#include <unordered_map>
#include <memory>
extern SDL::Renderer *renderer;
namespace Arcollect {
	namespace db {
		typedef sqlite_int64 account_id;
		/** Account data class holder
		 *
		 * This class hold data about an account. It's used trough a std::shared_ptr
		 * to maintain one instance per account.
		 *
		 * It is instanciated on-demand trough Arcollect::db::account::query().
		 */
		class account {
			private:
				std::shared_ptr<SDL::Texture> icon;
				account(Arcollect::db::account_id arcoid);
				// Cached DB infos
				sqlite_int64 data_version = -1;
				void db_sync(void);
				std::string acc_name;
				std::string acc_title;
				std::string acc_url;
			public:
				// Delete copy constructor
				account(const account&) = delete;
				account& operator=(account&) = delete;
				/** The database account id
				 */
				const Arcollect::db::account_id arcoid;
				
				inline const std::string &name(void) {
					db_sync();
					return acc_name;
				}
				inline const std::string &title(void) {
					db_sync();
					return acc_title;
				}
				inline const std::string &url(void) {
					db_sync();
					return acc_url;
				}
				inline std::shared_ptr<SDL::Texture> get_icon(void) {
					return icon;
				}
				/** Query an account
				 * \param arcoid The account identifier
				 * \return The account wrapped in a std::shared_ptr
				 * This function create or return a cached version of the #Arcollect::db::account.
				 */
				static std::shared_ptr<account> &query(Arcollect::db::account_id arcoid);
		};
	}
}
