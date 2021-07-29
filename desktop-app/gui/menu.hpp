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
#include "font.hpp"
#include "modal.hpp"
#include <functional>
#include <memory>
#include <vector>
namespace Arcollect {
	namespace gui {
		class menu_item {
			public:
				/** Standard menu item height
				 */
				static const int standard_height;
				virtual SDL::Point size(void) = 0;
				/** Process events
				 * \param e The event
				 * \param event_location   The rect that is sensitive to event (with padding)
				 * \param render_location  The rect where rendering occured (without padding)
				 */
				virtual void event(SDL::Event &e, const SDL::Rect &event_location, const SDL::Rect &render_location) = 0;
				virtual void render(SDL::Rect target) = 0;
				virtual ~menu_item(void) = default;
		};
		
		class menu: public modal {
			protected:
				std::vector<SDL::Rect> menu_rects;
				/** Anchor distance
				 *
				 * Each direction have a special meaning depending on which anchor is 
				 * enabled.
				 *
				 * If  anchor_top &&  anchor_bot, anchor_distance is the distance to
				 * both top and bottom borders.
				 * If  anchor_top && !anchor_bot, anchor_distance is the distance to
				 * the top border.
				 * If !anchor_top &&  anchor_bot, anchor_distance is the distance to
				 * the bottom border.
				 * If !anchor_top && anchor_bot, anchor_distance is ignored and the menu
				 * is centered
				 */
				SDL::Point anchor_distance;
			public:
				bool anchor_top;
				bool anchor_left;
				bool anchor_bot;
				bool anchor_right;
				
				SDL::Point padding{8,4};
				std::vector<std::shared_ptr<menu_item>> menu_items;
				
				bool event(SDL::Event &e) override;
				void render(void) override;
				int get_menu_item_at(SDL::Point cursor);
				int hovered_cell = -1;
				
				static void popup_context(const std::vector<std::shared_ptr<menu_item>> &menu_items, SDL::Point at, bool anchor_top = true, bool anchor_left = true, bool anchor_bot = false,  bool anchor_right = false);
				static unsigned int popup_context_count;
		};
		
		class menu_item_simple_label: public menu_item {
			private:
				bool pressed = false;
			protected:
				Arcollect::gui::font::Renderable text_line;
			public:
				SDL::Point size(void) override;
				void event(SDL::Event &e, const SDL::Rect &event_location, const SDL::Rect &render_location) override;
				void render(SDL::Rect target) override;
				std::function<void()> onclick;
				menu_item_simple_label(const char* label, std::function<void()> onclick);
		};
	}
}
