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
#pragma once
#include "font.hpp"
#include "modal.hpp"
namespace Arcollect {
	namespace gui {
		/** The about window
		 */
		class about_window: public modal {
			private:
				std::unique_ptr<Arcollect::gui::font::Renderable> render_cache;
				int cache_window_width;
				bool event(SDL::Event &e) override;
				std::vector<std::shared_ptr<Arcollect::gui::menu_item>> top_menu(void) override;
				void render() override;
			public:
				static void show(void);
		};
		extern about_window about_window_modal;
	}
}
