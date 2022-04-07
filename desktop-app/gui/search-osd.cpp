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
#include "../db/search.hpp"
#include "../i18n.hpp"

Arcollect::gui::search_osd Arcollect::gui::search_osd_modal;

bool Arcollect::gui::search_osd::event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx)
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
					update_background(saved_text);
					pop();
				} break;
				case SDL_SCANCODE_RETURN: {
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
void Arcollect::gui::search_osd::render(Arcollect::gui::modal::render_context render_ctx)
{
	render_titlebar(render_ctx);
}
void Arcollect::gui::search_osd::render_titlebar(Arcollect::gui::modal::render_context render_ctx)
{
	// Render text
	const int title_border = render_ctx.titlebar_target.h/4;
	text_render.render_tl(title_border+render_ctx.titlebar_target.h,title_border);
}
void Arcollect::gui::search_osd::text_changed(void)
{
	// Parse the search
	std::string_view search_term = " ";
	if (!text.empty())
		search_term = text;
	Arcollect::search::ParsedSearch search(std::string(search_term),Arcollect::db::SEARCH_ARTWORKS,Arcollect::db::SORT_RANDOM);
	// Update GUI
	const int title_border = window_borders::title_height/4;
	const int font_height = window_borders::title_height-2*title_border;
	font::RenderConfig render_config;
	render_config.base_font_height = font_height;
	text_render = font::Renderable(search.elements(),render_config);
	
	// Perform search
	Arcollect::gui::update_background(text);
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

std::vector<std::shared_ptr<Arcollect::gui::menu_item>> Arcollect::gui::search_osd::top_menu(void)
{
	using Arcollect::gui::menu_item_simple_label;
	if (!text.empty()) {
		return {
			std::make_shared<menu_item_simple_label>(i18n_desktop_app.edit_searching_artworks,std::bind(Arcollect::gui::popup_edit_art_metadata,
				Arcollect::search::ParsedSearch(text,Arcollect::db::SEARCH_ARTWORKS,Arcollect::db::SORT_NONE).make_shared_collection()
			)),
		};
	} else return {};
};
