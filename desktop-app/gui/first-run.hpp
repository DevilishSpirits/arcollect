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
		class first_run: public modal {
			std::unique_ptr<Arcollect::gui::font::Renderable> render_cache;
			int cache_window_width = -10;
			bool event(SDL::Event &e, SDL::Rect target) override;
			void render(SDL::Rect target) override;
		};
		extern first_run first_run_modal;
	}
}
