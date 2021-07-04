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
#include "../db/filter.hpp"
#include "../db/search.hpp"
#include "artwork-collections.hpp"
#include "menu.hpp"
#include "search-osd.hpp"
#include "slideshow.hpp"
#include "window-borders.hpp"
#include <arcollect-debug.hpp>

static std::function<std::unique_ptr<SQLite3::stmt>(void)> dynamic_update_background_func;
static bool dynamic_update_background_collection;
static sqlite_int64 slideshow_data_version;
static unsigned int slideshow_filter_version;

static class background_vgrid: public Arcollect::gui::view_vgrid {
	Arcollect::gui::artwork_viewport *mousedown_viewport;
	bool event(SDL::Event &e) override {
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN: {
				mousedown_viewport = get_pointed({e.button.x,e.button.y});
			} return false;
			case SDL_MOUSEBUTTONUP: {
				if (mousedown_viewport && (mousedown_viewport == get_pointed({e.button.x,e.button.y}))) {
					Arcollect::gui::background_slideshow.set_collection_iterator(*(mousedown_viewport->iter));
					Arcollect::gui::modal_stack.pop_back();
				}
			} return false;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_RETURN:
				case SDL_SCANCODE_ESCAPE: {
					Arcollect::gui::modal_stack.pop_back();
				} break;
				case SDL_SCANCODE_SPACE: {
					Arcollect::gui::search_osd_modal.push();
				} break;
				default: {
				} break;
			}
		} break;
			default:break;
		}
		return Arcollect::gui::view_vgrid::event(e);
	}
} background_vgrid;

Arcollect::gui::view_vgrid &Arcollect::gui::background_vgrid = ::background_vgrid;

static void delete_art(void);
static void confirm_delete_art(void)
{
	Arcollect::gui::menu::popup_context({
		std::make_shared<Arcollect::gui::menu_item_simple_label>("I really want to delete this artwork",::delete_art)
	},{0,Arcollect::gui::window_borders::title_height});
}
#ifdef ARTWORK_HAS_OPEN_URL
static void open_in_browser(void);
#endif

static class background_slideshow: public Arcollect::gui::view_slideshow {
	public:
	void delete_art(void) {
		viewport.artwork->db_delete();
	}
	#ifdef ARTWORK_HAS_OPEN_URL
	void open_in_browser(void) {
		viewport.artwork->open_url();
	}
	#endif
	bool event(SDL::Event &e) override {
		switch (e.type) {
			case SDL_KEYUP: {
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_RETURN:
					case SDL_SCANCODE_ESCAPE: {
						background_vgrid.resize(rect);
						Arcollect::gui::modal_stack.push_back(background_vgrid);
					} break;
					case SDL_SCANCODE_SPACE: {
						Arcollect::gui::search_osd_modal.push();
					} break;
					default:break;
				}
			} break;
			default:break;
		};
		return Arcollect::gui::view_slideshow::event(e);
	}
	std::vector<std::shared_ptr<Arcollect::gui::menu_item>> top_menu(void) override {
		if (viewport.artwork) {
			return {
				#ifdef ARTWORK_HAS_OPEN_URL
				std::make_shared<Arcollect::gui::menu_item_simple_label>("Browse...",::open_in_browser),
				#endif
				std::make_shared<Arcollect::gui::menu_item_simple_label>("Delete artwork",confirm_delete_art),
			};
		} else return {};
	};
	void render(void) override {
		// Regenerate collection on-demand
		if (dynamic_update_background_func && ((slideshow_data_version != Arcollect::data_version)||(slideshow_filter_version != Arcollect::db_filter::version))) {
			// Update version
			slideshow_data_version = Arcollect::data_version;
			slideshow_filter_version = Arcollect::db_filter::version;
			// Regenerate the stmt
			std::unique_ptr<SQLite3::stmt> stmt = dynamic_update_background_func();
			Arcollect::gui::update_background(stmt,dynamic_update_background_collection);
		}
		Arcollect::gui::view_slideshow::render();
	}
} background_slideshow;

static void delete_art(void)
{
	background_slideshow.delete_art();
}
#ifdef ARTWORK_HAS_OPEN_URL
static void open_in_browser(void)
{
	background_slideshow.open_in_browser();
}
#endif

Arcollect::gui::view_slideshow &Arcollect::gui::background_slideshow = ::background_slideshow;

void Arcollect::gui::update_background(Arcollect::db::artwork_id artid)
{
	std::shared_ptr<Arcollect::gui::artwork_collection> collection = std::make_shared<Arcollect::gui::artwork_collection_single>(artid);
	background_slideshow.set_collection(collection);
	background_vgrid.set_collection(collection);
}
void Arcollect::gui::update_background(std::unique_ptr<SQLite3::stmt> &stmt, bool collection)
{
	if (collection) {
		std::shared_ptr<Arcollect::gui::artwork_collection> new_collection = std::make_shared<Arcollect::gui::artwork_collection_sqlite>(stmt);
		background_slideshow.set_collection(new_collection);
		background_vgrid.set_collection(new_collection);
	} else if (stmt->step() == SQLITE_ROW)
		update_background(stmt->column_int64(0));
}

void Arcollect::gui::update_background(std::function<std::unique_ptr<SQLite3::stmt>(void)> &stmt_gen, bool collection)
{
	dynamic_update_background_func = stmt_gen;
	dynamic_update_background_collection = collection;
	// Force set_collection on next render()
	slideshow_data_version = -1;
	slideshow_filter_version = -1;
}

static std::string current_background_search;
static std::function<std::unique_ptr<SQLite3::stmt>(void)> default_update_background_generator = [](void) -> std::unique_ptr<SQLite3::stmt> {
	std::unique_ptr<SQLite3::stmt> stmt;
	Arcollect::db::search::build_stmt(current_background_search.c_str(),stmt);
	return stmt;
};
void Arcollect::gui::update_background(const std::string &search, bool collection)
{
	current_background_search = search;
	update_background(default_update_background_generator,collection);
	if (Arcollect::debug::is_on("search")) {
		std::vector<std::string_view> query_params;
		Arcollect::db::search::build_stmt(current_background_search.c_str(),std::cerr,query_params);
		std::cerr << std::endl;
		for (std::string_view &param: query_params)
			std::cerr << param << std::endl;
		std::cerr << std::endl;
	}
}

void Arcollect::gui::update_background(bool collection)
{
	update_background("",collection);
}

const std::string Arcollect::gui::get_current_search(void)
{
	return current_background_search;
}
