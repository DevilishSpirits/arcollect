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
#include "search-osd.hpp"
#include "edit-art.hpp"
#include "slideshow.hpp"
#include "menu.hpp"
#include "window-borders.hpp"
#include "../db/artwork-collections.hpp"
#include "../db/filter.hpp"
#include "../db/search.hpp"

Arcollect::gui::search_osd Arcollect::gui::search_osd_modal;

bool Arcollect::gui::search_osd::event(SDL::Event &e, SDL::Rect target)
{
	switch (e.type) {
		case SDL_WINDOWEVENT: {
		} return true;
		case SDL_MOUSEBUTTONDOWN: {
			pop();
		} return true;
		case SDL_KEYDOWN: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_BACKSPACE: {
					// Delete char
					if (!text.empty()) {
						if (text.back() & 0x80)
							// Remove all UTF-8 code points
							while ((text.back() & 0xC0) == 0x80)
								text.pop_back();
						text.pop_back();
					}
					text_changed();
				} break;
				default: {
				} break;
			}
		} return false;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_ESCAPE: {
					update_background(saved_text,true);
					pop();
				} break;
				case SDL_SCANCODE_RETURN: {
					update_background(text,true);
					pop();
				} break;
				default: {
				} break;
			}
		} return false;
		case SDL_TEXTINPUT: {
			text += std::string(e.text.text);
			text_changed();
		} return false;
		default: {
		} return false;
	}
}
void Arcollect::gui::search_osd::render(SDL::Rect target)
{
	// Update upon local changes only
	if ((private_data_version != Arcollect::private_data_version)||(filter_version != Arcollect::db_filter::version))
		text_changed();
	// Force render the title bar
	SDL::Point screen_size;
	renderer->GetOutputSize(screen_size);
	render_titlebar({0,0,screen_size.x,Arcollect::gui::window_borders::title_height},screen_size.x);
}
void Arcollect::gui::search_osd::render_titlebar(SDL::Rect target, int window_width)
{
	// Render text
	const int title_border = target.h/4;
	text_render.render_tl(title_border+target.h,title_border);
}
void Arcollect::gui::search_osd::text_changed(void)
{
	// Update GUI
	const int title_border = window_borders::title_height/4;
	const int font_height = window_borders::title_height-2*title_border;
	std::string_view search_term = " ";
	if (!text.empty())
		search_term = text;
	text_render = font::Renderable(font::Elements::build(font::ExactFontSize(font_height),search_term));
	
	// Perform search
	std::unique_ptr<SQLite3::stmt> stmt;
	Arcollect::db::search::build_stmt(search_term.data(),stmt);
	collection = std::make_shared<Arcollect::db::artwork_collection_sqlite>(std::move(stmt));
	Arcollect::gui::update_background(collection);
	
	// Update filter versions
	private_data_version = Arcollect::private_data_version;
	filter_version = Arcollect::db_filter::version;
}
void Arcollect::gui::search_osd::push(void)
{
	text = saved_text = Arcollect::gui::get_current_search();
	text_changed();
	SDL_StartTextInput();
	modal_stack.push_back(*this);
}
void Arcollect::gui::search_osd::pop(void)
{
	SDL_StopTextInput();
	to_pop = true;
}
void Arcollect::gui::search_osd::db_delete(void)
{
	if (!text.empty()) {
		std::unique_ptr<SQLite3::stmt> stmt;
		Arcollect::db::search::build_stmt(text.c_str(),stmt);
		Arcollect::db::artwork_collection_sqlite(std::move(stmt)).db_delete();
	}
}

std::vector<std::shared_ptr<Arcollect::gui::menu_item>> Arcollect::gui::search_osd::top_menu(void)
{
	using Arcollect::gui::menu_item_simple_label;
	if (!text.empty()) {
		return {
			std::make_shared<menu_item_simple_label>(U"Edit listed artworks…"s,std::bind(Arcollect::gui::popup_edit_art_metadata,collection)),
		};
	} else return {};
};
