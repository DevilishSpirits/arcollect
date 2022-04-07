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
#include "../db/search.hpp"
#include <string>
namespace Arcollect {
	namespace gui {
		class search_osd: public modal {
			private:
				std::string text;
				std::string saved_text;
				font::Renderable text_render;
				Arcollect::db::SearchType search_type = Arcollect::db::SEARCH_ARTWORKS;
				void text_changed(void);
				void pop(void);
			public:
				bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override;
				void render(Arcollect::gui::modal::render_context render_ctx) override;
				void render_titlebar(Arcollect::gui::modal::render_context render_ctx) override;
				std::vector<std::shared_ptr<Arcollect::gui::menu_item>> top_menu(void) override;
				void push(void);
		};
		extern search_osd search_osd_modal;
	}
}
