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
#include <config.h>
#include <functional>
#include <memory>
#include <vector>
namespace Arcollect {
	namespace gui {
		class menu_item {
			public:
				/** Menu render context
				 *
				 * Same role as #Arcollect::gui::modal::render_context but unlike this
				 * one the context is usually read-only and have a more specialized set
				 * of fields.
				 */
				struct render_context {
					SDL::Renderer &renderer;
					/** Rectangle of focus sensitivity
					 */
					SDL::Rect event_target;
					/** Rectangle of rendering
					 *
					 * Include a small padding.
					 */
					SDL::Rect render_target;
					/** Rectangle of the whole menu
					 *
					 * For use by Arcollect::gui::menu itself.
					 */
					SDL::Rect menu_rect;
					/** Weather the menu item have the focus
					 *
					 * Arcollect control the focus. Use this value, don't guess and fail.
					 */
					bool has_focus;
				};
				/** Standard menu item height
				 */
				static const int standard_height;
				virtual SDL::Point size(void) = 0;
				/** Process events
				 * \param e The event
				 * \param render_ctx The render context
				 */
				virtual bool event(SDL::Event &e, const render_context& render_ctx) = 0;
				virtual void render(const render_context& render_ctx) = 0;
				virtual ~menu_item(void) = default;
		};
		
		class menu: public modal {
			protected:
				using menu_item_render_context = menu_item::render_context;
				/** Make an Arcollect::gui::menu_item::render_context to the first item.
				 */
				menu_item_render_context begin_render_context(const Arcollect::gui::modal::render_context &render_ctx);
				/** Go to the next item from the begin_render_context() result
				 * \todo Document how it should be used (see render()/event())
				 */
				void step_render_context(menu_item_render_context& context, const std::shared_ptr<menu_item> &item);
			public:
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
				bool anchor_top;
				bool anchor_left;
				bool anchor_bot;
				bool anchor_right;
				SDL::Point padding{8,4};
				
				bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override;
				void render(Arcollect::gui::modal::render_context render_ctx) override;
				/** Menu items
				 */
				std::vector<std::shared_ptr<menu_item>> menu_items;
				/** Menu item with the focus
				 */
				std::shared_ptr<menu_item> focused_cell;
				
				static void popup_context(const std::vector<std::shared_ptr<menu_item>> &menu_items, SDL::Point at, bool anchor_top = true, bool anchor_left = true, bool anchor_bot = false,  bool anchor_right = false);
				static unsigned int popup_context_count;
		};
		
		/** Menu item that show a label and can be clicked
		 */
		class menu_item_label: public menu_item {
			private:
				bool pressed = false;
			protected:
				Arcollect::gui::font::Renderable text_line;
			public:
				virtual void clicked(void) = 0;
				SDL::Point size(void) override;
				bool event(SDL::Event &e, const render_context& render_ctx) override;
				void render(const render_context& render_ctx) override;
				menu_item_label(const font::Elements& elements);
		};
		
		class menu_item_simple_label: public menu_item_label {
			private:
				bool pressed = false;
			protected:
				Arcollect::gui::font::Renderable text_line;
			public:
				std::function<void()> onclick;
				void clicked(void) override {
					return onclick();
				}
				menu_item_simple_label(const font::Elements& elements, std::function<void()> onclick)
					: menu_item_label(elements), onclick(onclick) {}
				menu_item_simple_label(std::u32string&& label, std::function<void()> onclick)
					: menu_item_simple_label(font::Elements::build(std::move(label)),onclick) {}
				menu_item_simple_label(const std::u32string_view& label, std::function<void()> onclick)
					: menu_item_simple_label(std::u32string(label),onclick) {}
				menu_item_simple_label(const std::string_view& label, std::function<void()> onclick)
					: menu_item_simple_label(font::Elements::build(label),onclick) {}
		};
		
		/** Menu item that open a website
		 *
		 * \warning Behavior is undefined if HAS_SDL_OPENURL is defined to 0!
		 */
		class menu_item_simple_link: public menu_item_label {
			public:
				const std::string_view link;
				#if HAS_SDL_OPENURL
				void clicked(void) override {
					SDL_OpenURL(link.data());
				}
				#endif
				menu_item_simple_link(const font::Elements& elements, const std::string_view &link)
					: menu_item_label(elements), link(link) {}
				menu_item_simple_link(const std::u32string_view& label, const std::string_view &link)
					: menu_item_simple_link(font::Elements::build(label),link) {}
				template <typename ... Args>
				static std::shared_ptr<menu_item> make_shared(Args... args) {
					return std::shared_ptr<menu_item>(new menu_item_simple_link(args...));
				}
		};
	}
}
