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
#include "../db/search.hpp"
#include "../db/artwork-collections.hpp"
#include "edit-art.hpp"
#include "menu.hpp"
#include "search-osd.hpp"
#include "slideshow.hpp"
#include "window-borders.hpp"
#include <arcollect-debug.hpp>

static std::unique_ptr<Arcollect::search::ParsedSearch> current_background_search(new Arcollect::search::ParsedSearch());
static sqlite_int64 slideshow_data_version;

static class background_vgrid: public Arcollect::gui::view_vgrid {
	Arcollect::gui::artwork_viewport *mousedown_viewport;
	bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override {
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN: {
				mousedown_viewport = get_pointed(render_ctx,{e.button.x,e.button.y});
			} return false;
			case SDL_MOUSEBUTTONUP: {
				if (mousedown_viewport && (mousedown_viewport == get_pointed(render_ctx,{e.button.x,e.button.y}))) {
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
		return Arcollect::gui::view_vgrid::event(e,render_ctx);
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
	bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override {
		switch (e.type) {
			case SDL_KEYUP: {
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_AC_HOME:
					case SDL_SCANCODE_RETURN:
					case SDL_SCANCODE_ESCAPE: {
						Arcollect::gui::modal_stack.push_back(background_vgrid);
						if (collection && (collection_iterator != collection->end()))
							background_vgrid.bring_to_view(render_ctx,Arcollect::db::artwork::query(*collection_iterator));
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
		return Arcollect::gui::view_slideshow::event(e,render_ctx);
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
	void render(Arcollect::gui::modal::render_context render_ctx) override {
		// Regenerate collection on-demand
		if (current_background_search && (slideshow_data_version != Arcollect::data_version)) {
			// Update version
			slideshow_data_version = Arcollect::data_version;
			// Regenerate the stmt
			std::shared_ptr<Arcollect::db::artwork_collection> new_collection = current_background_search->make_shared_collection();
			Arcollect::gui::update_background(new_collection);
		}
		switch (viewport.download->artwork_type) {
			case ARTWORK_TYPE_IMAGE: {
				if (viewport.download) {
					renderer->SetDrawColor(viewport.download->background_color);
					renderer->FillRect(render_ctx.target);
				}
			}
			default:break;
		}
		Arcollect::gui::view_slideshow::render(render_ctx);
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
}

void Arcollect::gui::update_background(const std::string &search)
{
	current_background_search = std::make_unique<Arcollect::search::ParsedSearch>(search,Arcollect::db::SEARCH_ARTWORKS,Arcollect::db::SORT_RANDOM);
	// Force set_collection on next render()
	slideshow_data_version = -1;
}

const std::string Arcollect::gui::get_current_search(void)
{
	return current_background_search->search;
}
