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
#include "sdl2-hpp/SDL.hpp"
#include <functional>
#include <memory>
#include <variant>
#include <vector>
namespace Arcollect {
	namespace gui {
		class menu_item;
		/** Something that can be modal
		 *
		 * Arcollect UI use a stack of #Arcollect::gui::modal objects to deliver
		 * event and rendering.
		 */
		class modal {
			public:
				/** Handle an event
				 * \param e The event
				 * \return Weather propagate event to the next element in the stack.
				 *         True to propagate the event.
				 */
				virtual bool event(SDL::Event &e) = 0;
				/** Render the object
				 *
				 * Render the object on the screen
				 */
				virtual void render() = 0;
				/** Render the title bar
				 * \param target       The rendering rect target
				 * \param window_width The window width for information
				 *
				 * Render the titlebar.
				 */
				virtual void render_titlebar(SDL::Rect target, int window_width);
				/** Get menu items for topbar context menu
				 * \return The list of modal dependant menus to add.
				 *
				 * This callback is invoked when the user click on the triangle menu on
				 * the top. Menus returned here are prepended to the menu shown.
				 */
				virtual std::vector<std::shared_ptr<menu_item>> top_menu(void) { return {}; };
				virtual ~modal(void) = default;
		};
		
		using modal_stack_variant = std::variant<
			std::reference_wrapper<modal>,
			std::unique_ptr<modal>,
			std::shared_ptr<modal>
		>;
		extern std::vector<modal_stack_variant> modal_stack;
		
		inline modal& modal_get(modal_stack_variant &value) {
			switch (value.index()) {
				case 0:return std::get<std::reference_wrapper<modal>>(value);
				case 1:return *std::get<std::unique_ptr<modal>>(value);
				case 2:return *std::get<std::shared_ptr<modal>>(value);
				default:return *reinterpret_cast<modal*>(NULL);
			}
		}
		inline modal& modal_get(decltype(modal_stack)::iterator &iter) {
			return modal_get(*iter);
		}
		inline modal& modal_get(decltype(modal_stack)::reverse_iterator &iter) {
			return modal_get(*iter);
		}
		inline modal& modal_front(void) {
			return modal_get(modal_stack.front());
		}
		inline modal& modal_back(void) {
			return modal_get(modal_stack.back());
		}
	}
}

