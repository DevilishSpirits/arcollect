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
#include "arcollect-i18n-desktop_app.hpp"
void Arcollect::i18n::desktop_app::apply_C(void) noexcept {
	edit_artwork_set_rating_confirm_ = [](const desktop_app &self, int rating, SDL::Color color) -> Elements {
		std::string rating_string;
		switch (rating) {
			case 0 :rating_string = self.rating_level_none;break;
			case 16:rating_string = self.rating_level_mature;break;
			case 18:rating_string = self.rating_level_adult;break;
			default:rating_string = "no one under " + std::to_string(static_cast<int>(rating));break;
		};
		return Elements::build(U"Mark as "sv,color,rating_string);
 };
}
