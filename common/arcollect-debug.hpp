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
#include <string_view>
namespace Arcollect {
	/** Debugging pseudo class
	 *
	 * This is a singleton class to ease debug flag readings.
	 * \warning **This class use black magic!**
	 *          It must ONLY contain Arcollect::Debug::Flag because it is used as
	 *          a Arcollect::Debug::Flag[]!
	 *          The cause may hurt your sensibility.
	 */
	extern struct Debug {
		struct Flag {
			/** The debug flag name
			 */
			const std::string_view name;
			/** If the flag is raised
			 */
			bool on = false;
			/** Convenience 
			 *
			 */
			operator bool(void) const {
				return on;
			}
			constexpr Flag(const std::string_view& name) : name(name) {}
		};
		using iterator = Flag*;
		/** The value of end()
		 *
		 * #Debug is a singleton anyway.
		 */
		static iterator end_ptr;
		/** Return an iterator to all debug #Flag
		 * \return Something fast to compute...
		 */
		iterator begin(void) {
			// No. This is not a dream.
			return reinterpret_cast<iterator>(this);
		}
		/** Return the end of begin()
		 * \return Litterally #end_ptr.
		 */
		iterator end(void) {
			// I am really doing this.
			return end_ptr;
		}
		/** Turn on a debugging flag by name.
		 * \param flag_name The flag name
		 */
		void turn_on_flag(const std::string_view& flag_name);
		Debug(void);
		
		Flag icc_profile{"icc-profile"};
		Flag redraws{"redraws"};
		Flag search{"search"};
		Flag webext_adder{"webext-adder"};
	} debug;
}
