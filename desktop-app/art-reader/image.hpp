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
#include <string_view>
#include "../sdl2-hpp/SDL.hpp"
namespace Arcollect {
	namespace art_reader {
		/** Load an image artwork
		 * \return A surface with pixels data, or NULL on error
		 */
		SDL::Surface *image(const std::filesystem::path &path);
		
		/** Set screen ICC profile
		 * \param icc_profile The ICC profile to read
		 *
		 * Replace the screen ICC profile.
		 */
		void set_screen_icc_profile(const std::string_view& icc_profile);
		/** Set screen ICC profile
		 * \param icc_profile The window to extract ICC profile from
		 *
		 * Replace the screen ICC profile.
		 */
		void set_screen_icc_profile(SDL_Window *window);
	};
}
