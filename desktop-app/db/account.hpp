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
#include <memory>
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
				account(Arcollect::db::account_id arcoid);
				// Cached DB infos
				sqlite_int64 data_version = -1;
				void db_sync(void);
				std::string acc_name;
				std::string acc_title;
				std::string acc_url;
				sqlite_int64 acc_icon;
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
				/** Query the account icon
				 * \return The icon texture or NULL if not available yet
				 */
				std::unique_ptr<SDL::Texture> &get_icon(void);
				/** Query an account
				 * \param arcoid The account identifier
				 * \return The account wrapped in a std::shared_ptr
				 * This function create or return a cached version of the #Arcollect::db::account.
				 */
				static std::shared_ptr<account> &query(Arcollect::db::account_id arcoid);
		};
	}
}
