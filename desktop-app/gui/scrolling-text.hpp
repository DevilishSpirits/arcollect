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
/** \file scrolling-text.hpp
 *  \brief Scrolling text toolkit
 */
#include "animation.hpp"
#include "font.hpp"
#include "modal.hpp"
namespace Arcollect {
	namespace gui {
		class scrolling_text: public modal {
			private:
				Arcollect::gui::font::Elements elements;
				std::unique_ptr<gui::font::Renderable> renderable;
				int renderable_target_width;
				animation::scrolling<int> scroll;
				void scroll_text(int line_delta, const SDL::Rect &rect);
			public:
				void set_static_elements(const Arcollect::gui::font::Elements& new_elements);
				void render(SDL::Rect target) override;
				bool event(SDL::Event &e, SDL::Rect target) override;
		};
	}
}
