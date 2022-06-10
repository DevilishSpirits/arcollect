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
#include "menu.hpp"
#include "../db/db.hpp"
#include "../db/account.hpp"
#include "../db/download.hpp"
#include <config.h>
namespace Arcollect {
	namespace gui {
		/** Data structure used to generate a menu_db_object_item.
		 */
		struct menu_db_object_item_info {
			/** The object title
			 */
			std::string title;
			/** The object subtitle
			 */
			std::string subtitle;
			/** Object thumbnail
			 *
			 */
			std::shared_ptr<Arcollect::db::download> thumbnail;
			
			menu_db_object_item_info(const std::shared_ptr<db::account> &account);
		};
		/** Artworks/account/many db entries item
		 * \param objT Type of the object stored (e.g. std::shared_ptr<Arcollect::db::account>).
		 *
		 * This kind of items takes two lines and display a thumbnail on the left.
		 */
		template <typename objT>
		class menu_db_object_item: public menu_item {
			public:
				/** The object we refer to.
				 */
				const objT object;
			private:
				/** The text to show
				 */
				Arcollect::gui::font::Renderable text_line;
				/** The thumbnail to render
				 */
				std::shared_ptr<Arcollect::db::download> thumbnail;
				sqlite3_int64 data_version = -1;
				void check_db_version(void) {
					if (data_version != Arcollect::data_version) {
						// Get things
						menu_db_object_item_info item_info(object);
						data_version = Arcollect::data_version;
						// Generate text
						Arcollect::gui::font::Elements elements;
						elements << item_info.title;
						if (!item_info.subtitle.empty())
							elements << U"\n"sv << SDL::Color{255,255,255,128} << item_info.subtitle;
						
						// Update renderables
						thumbnail = item_info.thumbnail;
						text_line = Arcollect::gui::font::Renderable(elements);
					}
				}
			public:
				using click_function = std::function<void(const objT&, const render_context& render_ctx)>;
				click_function onclick;
				#if HAS_SDL_OPENURL
				/** Action to browse to the account
				 */
				static void click_function_browse(const std::shared_ptr<Arcollect::db::account> &account, const render_context&) {
					SDL_OpenURL(account->url().c_str());
				}
				#endif
				/** Dummy action
				 */
				static void click_function_default(const objT& object, const render_context& render_ctx) {
					#if HAS_SDL_OPENURL
						return click_function_browse(object,render_ctx);
					#endif
				}
				/** Return the default icon size
				 */
				static int get_icon_size(void) {
					return Arcollect::gui::font::RenderConfig().base_font_height*2;
				}
				SDL::Point size(void) override {
					check_db_version(); // It's better to check this there
					SDL::Point size = text_line.size();
					auto icon_size = get_icon_size();
					size.x += icon_size+4;
					size.y = std::max(size.y,icon_size);
					return size;
				}
				bool event(SDL::Event &e, const render_context& render_ctx) override {
					switch (e.type) {
						case SDL_MOUSEBUTTONUP: {
							if (render_ctx.has_focus) {
								onclick(object,render_ctx);
								return false;
							}
						} break;
					}
					return true;
				}
				void render(const render_context& render_ctx) override {
					const auto &render_target = render_ctx.render_target; 
					//check_db_version(); // This is checked in size()
					auto icon_size = get_icon_size();
					std::unique_ptr<SDL::Texture> &icon = thumbnail->query_image();
					if (icon) {
						SDL::Rect rect{render_target.x,render_target.y + (render_target.h-icon_size)/2,icon_size,icon_size};
						render_ctx.renderer.Copy(icon.get(),NULL,&rect);
					}
					text_line.render_cl(render_target.x + icon_size + 4,render_target.y,render_target.h);
				}
				menu_db_object_item(const objT &object, click_function onclick = click_function_default) : object(object), onclick(onclick) {}
		};
		using menu_account_item = menu_db_object_item<std::shared_ptr<Arcollect::db::account>>;
	}
}
