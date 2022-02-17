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
#include "../db/download.hpp"
#include <variant>
namespace Arcollect {
	namespace gui {
		class scrolling_text: public modal {
			private:
				std::variant<
					Arcollect::gui::font::Elements,
					std::shared_ptr<Arcollect::db::download>
				> data;
				/** Check if an #Arcollect::gui::font::Elements is available
				 * \return true if so
				 *
				 * It is used to check if it is safe to call get_elements()
				 */
				bool elements_available(void) const;
				/** Query elements
				 * \return A reference to the elements
				 *
				 * \warning This function is only valid if the elements objects is
				 *          available! Always ensure that elements_available() is true
				 *          before calling this function.
				 */
				Arcollect::gui::font::Elements& get_elements(void);
				std::unique_ptr<gui::font::Renderable> renderable;
				int renderable_target_width;
				animation::scrolling<int> scroll;
				void scroll_text(int line_delta, const SDL::Rect &rect);
			public:
				template <typename T>
				void set(const T& new_elements) {
					data = new_elements;
					renderable.reset();
				}
				void render(Arcollect::gui::modal::render_context render_ctx) override;
				bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override;
		};
	}
}
