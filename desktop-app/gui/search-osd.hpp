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
#include "font.hpp"
#include "modal.hpp"
#include "../db/artwork-collection.hpp"
#include <string>
namespace Arcollect {
	namespace gui {
		class search_osd: public modal {
			private:
				std::string text;
				std::string saved_text;
				font::Renderable text_render;
				void text_changed(void);
				void pop(void);
				std::shared_ptr<Arcollect::db::artwork_collection> collection;
			public:
				bool event(SDL::Event &e, SDL::Rect target) override;
				void render(SDL::Rect target) override;
				void render_titlebar(SDL::Rect target, int window_width) override;
				std::vector<std::shared_ptr<Arcollect::gui::menu_item>> top_menu(void) override;
				void push(void);
		};
		extern search_osd search_osd_modal;
	}
}
