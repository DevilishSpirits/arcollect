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
#include "menu-db-object.hpp"
#include "window-borders.hpp"
#include "../db/artwork-collections.hpp"
#include "../db/search.hpp"
#include "../i18n.hpp"

Arcollect::gui::search_osd Arcollect::gui::search_osd_modal;
static Arcollect::gui::menu autocompletion_menu = []() {
	Arcollect::gui::menu autocompletion_menu;
	autocompletion_menu.anchor_distance = {0,64};
	autocompletion_menu.anchor_top = true;
	autocompletion_menu.anchor_left = true;
	autocompletion_menu.anchor_bot = false;
	autocompletion_menu.anchor_right = false;
	return autocompletion_menu;
}();
// Flag raised when Arcollect::gui::search_osd::event should call text_changed()
static bool text_has_changed;

bool Arcollect::gui::search_osd::event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx)
{
	bool result;
	text_has_changed = false;
	switch (e.type) {
		case SDL_WINDOWEVENT: {
			result = true;
		} break;
		case SDL_MOUSEBUTTONDOWN: {
			if (autocompletion_menu.focused_cell)
				result = false;
			else {
				pop();
				result = true;
			}
		} break;
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
					text_has_changed = true;
				} break;
				default: {
				} break;
			}
			result = false;
		} break;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_ESCAPE: {
					update_background(saved_text);
					pop();
				} break;
				case SDL_SCANCODE_RETURN: {
					if (autocompletion_menu.focused_cell) {
						Arcollect::gui::menu_account_item &cell = static_cast<Arcollect::gui::menu_account_item&>(*autocompletion_menu.focused_cell);
						cell.onclick(cell.object,Arcollect::gui::menu_item::render_context(render_ctx.renderer));
					} else pop();
				} break;
				default: {
				} break;
			}
			result = false;
		} break;
		case SDL_TEXTINPUT: {
			text += std::string(e.text.text);
			text_has_changed = true;
			result = false;
		} break;
		default: {
			result = false;
		} break;
	}
	result &= autocompletion_menu.event(e,render_ctx) && result;
	if (text_has_changed)
		text_changed();
	return result;
}
void Arcollect::gui::search_osd::render(Arcollect::gui::modal::render_context render_ctx)
{
	render_titlebar(render_ctx);
	if (!autocompletion_menu.menu_items.empty())
		autocompletion_menu.render(render_ctx);
}
void Arcollect::gui::search_osd::render_titlebar(Arcollect::gui::modal::render_context render_ctx)
{
	// Render text
	const int title_border = render_ctx.titlebar_target.h/4;
	text_render.render_tl(title_border+render_ctx.titlebar_target.h,title_border);
}
void Arcollect::gui::search_osd::text_changed(void)
{
	std::unique_ptr<SQLite3::stmt> stmt;
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
	
	// Perform auto-completion
	using Arcollect::search::AutoCompleteMode;
	autocompletion_menu.menu_items.clear();
	autocompletion_menu.focused_cell.reset();
	switch (search.auto_complete(stmt)) {
		case AutoCompleteMode::AUTOCOMP_NONE: {
			// Do nothing
		} break;
		case AutoCompleteMode::AUTOCOMP_ACCOUNT: {
			Arcollect::gui::menu_account_item::click_function onclick = [this](const std::shared_ptr<Arcollect::db::account>&account, const Arcollect::gui::menu_item::render_context&) {
					text.erase(text.find_last_of(':')+1);
					text.append(account->name());
					text += ' ';;
					text_has_changed = true;
				};
			while (stmt->step() == SQLITE_ROW)
				autocompletion_menu.menu_items.push_back(std::make_shared<Arcollect::gui::menu_account_item>(Arcollect::db::account::query(stmt->column_int(0)),onclick));
		} break;
	}
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
