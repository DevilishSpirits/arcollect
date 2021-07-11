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
#include "modal.hpp"
#include <string>
namespace Arcollect {
	namespace gui {
		class search_osd: public modal {
			private:
				std::string text;
				std::string saved_text;
				/** Flag to check if we should ls poop the vgrid
				 *
				 * Upon push(), #Arcollect::gui::background_vgrid is also pushed is not.
				 * This flag is set to remember weather the grid was shown or not.
				 */
				bool also_pop_grid_after;
				void pop(void);
			public:
				bool event(SDL::Event &e) override;
				void render(void) override;
				void render_titlebar(SDL::Rect target, int window_width) override;
				void push(void);
		};
		extern search_osd search_osd_modal;
	}
}
