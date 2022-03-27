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
#include <arcollect-i18n.hpp>
#include "../desktop-app/gui/font.hpp"
#include <../dependency-report.hpp> // For EMBEDED_DEPENDENCIES_COUNT
namespace Arcollect {
	namespace i18n {
		struct desktop_app {
			using Elements = Arcollect::gui::font::Elements;
			#if EMBEDED_DEPENDENCIES_COUNT > 0
			/** Listing of embeded dependencies for use within the about modal
			 *
			 * \warning It is not defined if `EMBEDED_DEPENDENCIES_COUNT` is zero.
			 */
			std::string_view about_this_embed_deps;
			#endif
			Elements (*edit_artwork_set_rating_confirm)(const desktop_app &self, int rating, SDL::Color color);
			#include "arcollect-i18n-desktop_app-struct-autogenerated.hpp"
		};
	}
}
