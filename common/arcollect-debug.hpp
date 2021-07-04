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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <string>
#include <string_view>
#include <unordered_set>
namespace Arcollect {
	namespace debug {
		/** Value of $ARCOLLECT_DEBUG environment variable
		 */
		extern const std::string env;
		/** Value of debug flags on
		 */
		extern const std::unordered_set<std::string_view> flags;
		/** Wheater all debug flags are raised
		 *
		 * This boolean is true when the 'all' debug flag is set and cause all flags
		 * to be raised.
		 */
		extern const bool all_flag;
		
		/** Check if a debug flag is on
		 */
		static bool is_on(const std::string_view &debug_flag) {
			return all_flag || (flags.find(debug_flag) != flags.end());
		}
	}
}
