/* Arcollect -- An artwork collection manager0
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
#include <../../dependency-report.hpp>
static constexpr void cpy_view(char base[], int &i, const std::string_view &view)
{
	for (char chr: view)
		base[i++] = chr;
}
void Arcollect::i18n::desktop_app::apply_fr(void) noexcept {
	#if EMBEDED_DEPENDENCIES_COUNT > 0
	about_this_embed_deps = []() /* TODO consteval */ -> std::string_view {
		// Okay. Let me explain the black-magic here.
		// I am just computing 
		constexpr std::string_view prefix = embeded_dependencies.size() > 1 ? "Cette copie d'Arcollect intègre des copies de " : "Cette copie d'Arcollect intègre une copie de ";
		static char store[prefix.size()
			+Arcollect::Dependency::compute_about_this_embed_deps_size(embeded_dependencies,3,1,3)
			+((embeded_dependencies.size() > 1) * 3)
			-(embeded_dependencies.size() == 1)
		];
		int cursor = 0;
		cpy_view(store,cursor,prefix);
		for (int i = embeded_dependencies.size()-1; i >= 0; --i) {
			const auto& current_dep = embeded_dependencies[i];
			// Print data
			cpy_view(store,cursor,current_dep.name);
			if (!current_dep.version.empty()) {
				store[cursor++] = ' ';
				cpy_view(store,cursor,current_dep.version);
			}
			if (!current_dep.website.empty()) {
				store[cursor++] = ' ';
				store[cursor++] = '(';
				cpy_view(store,cursor,current_dep.website);
				store[cursor++] = ')';
			}
			// Print separator
			std::string_view after;
			switch (i) {
				default:after = ", ";break;
				case 1:after = " et ";break;
				case 0:after = ".";break;
			}
			cpy_view(store,cursor,after);
			//static_assert(cursor == sizeof(prefix),"store size is too big");
		}
		return std::string_view(store,cursor);
	}();
	#endif
	edit_artwork_set_rating_confirm = [](const desktop_app &self, int rating, SDL::Color color)-> Elements {
		std::string rating_string;
		switch (rating) {
			case 0 :rating_string = self.rating_level_none;break;
			case 16:rating_string = self.rating_level_mature;break;
			case 18:rating_string = self.rating_level_adult;break;
			default:rating_string = "déconseillé au moins de " + std::to_string(static_cast<int>(rating)) + " ans";break;
		};
		return Elements::build(U"Marquer comme contenu "sv,color,rating_string);
 };
}
