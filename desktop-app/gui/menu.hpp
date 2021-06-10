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
/** \file menu.hpp
 *  \brief Menu toolkit
 *
 * The "menu" toolkit is heavily used in the GUI. Even within configurations
 * window. In fact this toolkit provide a simple way to organize items in a
 * vertical layout and hence, is reusable in many places.
 */
#pragma once
#include "modal.hpp"
#include "font.hpp"
#include <functional>
#include <memory>
#include <vector>
namespace Arcollect {
	namespace gui {
		class menu_item {
			protected:
				Arcollect::gui::Font font;
			public:
				/** Standard menu item height
				 */
				static const int standard_height;
				virtual SDL::Point size(void) = 0;
				virtual void event(SDL::Event &e, SDL::Rect location) = 0;
				virtual void render(SDL::Rect target) = 0;
				virtual ~menu_item(void) = default;
		};
		
		class menu: public modal {
			protected:
				std::vector<SDL::Rect> menu_rects;
			public:
				SDL::Point topleft;
				SDL::Point padding{8,4};
				std::vector<std::shared_ptr<menu_item>> menu_items;
				
				bool event(SDL::Event &e) override;
				void render(void) override;
				void render_titlebar(SDL::Rect target, int window_width) override {};
				int get_menu_item_at(SDL::Point cursor);
				int hovered_cell = -1;
				
				static void popup_context(const std::vector<std::shared_ptr<menu_item>> &menu_items, SDL::Point at);
		};
		
		class menu_item_simple_label: public menu_item {
			private:
				bool pressed = false;
			protected:
				Arcollect::gui::TextLine text_line;
				std::unique_ptr<SDL::Texture> text;
			public:
				SDL::Point size(void) override;
				void event(SDL::Event &e, SDL::Rect location) override;
				void render(SDL::Rect target) override;
				std::function<void()> onclick;
				menu_item_simple_label(const char* label, std::function<void()> onclick);
		};
	}
}
