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
#include "../i18n.hpp"
#include "../db/filter.hpp"
#include "../db/search.hpp"
#include "../db/artwork-collections.hpp"
#include "edit-art.hpp"
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
	bool event(SDL::Event &e, SDL::Rect target) override {
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN: {
				mousedown_viewport = get_pointed({e.button.x,e.button.y});
			} return false;
			case SDL_MOUSEBUTTONUP: {
				if (mousedown_viewport && (mousedown_viewport == get_pointed({e.button.x,e.button.y}))) {
					Arcollect::gui::background_slideshow.target_artwork = mousedown_viewport->artwork;
					Arcollect::gui::background_slideshow.set_collection_iterator(*(mousedown_viewport->iter));
					to_pop = true;
				}
			} return false;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_RETURN:
				case SDL_SCANCODE_ESCAPE: {
					to_pop = true;
				} break;
				case SDL_SCANCODE_AC_SEARCH:
				case SDL_SCANCODE_FIND:
				case SDL_SCANCODE_SPACE: {
					Arcollect::gui::search_osd_modal.push();
				} break;
				default: {
				} break;
			}
		} break;
			default:break;
		}
		return Arcollect::gui::view_vgrid::event(e,target);
	}
} background_vgrid;

Arcollect::gui::view_vgrid &Arcollect::gui::background_vgrid = ::background_vgrid;

#ifdef ARTWORK_HAS_OPEN_URL
static void open_in_browser(void);
#endif

static class background_slideshow: public Arcollect::gui::view_slideshow {
	public:
	#ifdef ARTWORK_HAS_OPEN_URL
	void open_in_browser(void) {
		viewport.artwork->open_url();
	}
	#endif
	bool event(SDL::Event &e, SDL::Rect target) override {
		switch (e.type) {
			case SDL_KEYUP: {
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_AC_HOME:
					case SDL_SCANCODE_RETURN:
					case SDL_SCANCODE_ESCAPE: {
						background_vgrid.resize(rect);
						Arcollect::gui::modal_stack.push_back(background_vgrid);
					} break;
					case SDL_SCANCODE_AC_SEARCH:
					case SDL_SCANCODE_FIND:
					case SDL_SCANCODE_SPACE: {
						Arcollect::gui::search_osd_modal.push();
					} break;
					#ifdef ARTWORK_HAS_OPEN_URL
					case SDL_SCANCODE_WWW: {
						// Open artwork in browser
						open_in_browser();
					} break;
					#endif
					default:break;
				}
			} break;
			default:break;
		};
		return Arcollect::gui::view_slideshow::event(e,target);
	}
	static void edit_art(background_slideshow *self) {
		using namespace Arcollect::db;
		std::shared_ptr<artwork_collection> collection = std::make_shared<artwork_collection_single>(self->viewport.artwork->art_id);
		Arcollect::gui::popup_edit_art_metadata(collection);
	}
	std::vector<std::shared_ptr<Arcollect::gui::menu_item>> top_menu(void) override {
		if (viewport.artwork) {
			return {
				#ifdef ARTWORK_HAS_OPEN_URL
				std::make_shared<Arcollect::gui::menu_item_simple_label>(Arcollect::i18n_desktop_app.browse_to_arwork_page,::open_in_browser),
				#endif
				std::make_shared<Arcollect::gui::menu_item_simple_label>(Arcollect::i18n_desktop_app.edit_current_artwork,std::bind(edit_art,this)),
			};
		} else return {};
	};
	void render(SDL::Rect target) override {
		// Regenerate collection on-demand
		if (dynamic_update_background_func && ((slideshow_data_version != Arcollect::data_version)||(slideshow_filter_version != Arcollect::db_filter::version))) {
			// Update version
			slideshow_data_version = Arcollect::data_version;
			slideshow_filter_version = Arcollect::db_filter::version;
			// Regenerate the stmt
			std::unique_ptr<SQLite3::stmt> stmt = dynamic_update_background_func();
			Arcollect::gui::update_background(std::move(stmt),dynamic_update_background_collection);
		}
		Arcollect::gui::view_slideshow::render(target);
	}
} background_slideshow;

#ifdef ARTWORK_HAS_OPEN_URL
static void open_in_browser(void)
{
	background_slideshow.open_in_browser();
}
#endif

Arcollect::gui::view_slideshow &Arcollect::gui::background_slideshow = ::background_slideshow;

void Arcollect::gui::update_background(std::shared_ptr<Arcollect::db::artwork_collection> &new_collection)
{
	background_slideshow.set_collection(new_collection);
	background_vgrid.set_collection(new_collection);
	dynamic_update_background_func = std::function<std::unique_ptr<SQLite3::stmt>(void)>();
}
void Arcollect::gui::update_background(std::unique_ptr<SQLite3::stmt> &&stmt, bool collection)
{
	if (collection) {
		std::shared_ptr<Arcollect::db::artwork_collection> new_collection = std::make_shared<Arcollect::db::artwork_collection_sqlite>(std::move(stmt));
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
	if (Arcollect::debug.search) {
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
