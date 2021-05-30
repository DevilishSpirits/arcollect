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
#include <filesystem>
#include "../sqlite3-hpp/sqlite3.hpp"
namespace Arcollect {
	namespace path {
		/** The XDG configuration home ($XDG_CONFIG_HOME)
		 */
		extern const std::filesystem::path xdg_config_home;
		/** The Arcollect data home ($XDG_DATA_HOME/arcollect/)
		 *
		 * This is were all datas are stored. Artworks and database included.
		 */
		extern const std::filesystem::path arco_data_home;
		
		/** The artwork pool location ($XDG_DATA_HOME/arcollect/artworks/)
		 */
		extern const std::filesystem::path artwork_pool;
		
		/** The account avatar pool location ($XDG_DATA_HOME/arcollect/account-avatars/)
		 */
		extern const std::filesystem::path account_avatars;
	}
}
