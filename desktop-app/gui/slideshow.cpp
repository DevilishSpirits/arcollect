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
#include "slideshow.hpp"
#include "artwork-collections.hpp"

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
				case SDL_SCANCODE_ESCAPE: {
					Arcollect::gui::modal_stack.pop_back();
				} break;
			 return false;
			}
		} break;
			default:break;
		}
		return Arcollect::gui::view_vgrid::event(e);
	}
} background_vgrid;

static class background_slideshow: public Arcollect::gui::view_slideshow {
	bool event(SDL::Event &e) override {
		switch (e.type) {
			case SDL_KEYUP: {
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_ESCAPE: {
						background_vgrid.resize(rect);
						Arcollect::gui::modal_stack.push_back(background_vgrid);
					} break;
					default:break;
				}
			} break;
			default:break;
		};
		return Arcollect::gui::view_slideshow::event(e);
	}
} background_slideshow;

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

void Arcollect::gui::update_background(bool collection)
{
	std::unique_ptr<SQLite3::stmt> stmt;
	database->prepare("SELECT art_artid FROM artworks WHERE "+db_filter::get_sql()+" ORDER BY random();",stmt);
	update_background(stmt,collection);
}
