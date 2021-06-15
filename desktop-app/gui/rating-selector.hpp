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
#pragma once
#include "../config.hpp"
#include "menu.hpp"
namespace Arcollect {
	namespace gui {
		class rating_selector {
			protected:
				/** General rect tool
				 * \param rect The target rectangle. It get resized to the right width and flushed rright
				 * \return A rect suitable to 
				 */
				SDL::Rect rect_tool(SDL::Rect &rect);
				Arcollect::config::Rating hover_rating;
			public:
				bool has_kid;
				bool has_pg13;
				bool has_mature;
				bool has_adult;
				
				Arcollect::config::Rating rating = Arcollect::config::Rating::RATING_ADULT;
				Arcollect::config::Rating pointed_rating(SDL::Rect target, SDL::Point cursor);
				
				std::function<void(Arcollect::config::Rating)> onratingset;
				
				void render(SDL::Rect target);
				void event(SDL::Event &e, SDL::Rect target);
		};
		class rating_selector_menu: public menu_item {
			private:
				Arcollect::gui::TextLine text_line;
			public:
				rating_selector selector;
				SDL::Point size(void) override;
				void event(SDL::Event &e, SDL::Rect location) override;
				void render(SDL::Rect target) override;
				rating_selector_menu(void);
		};
	}
}
