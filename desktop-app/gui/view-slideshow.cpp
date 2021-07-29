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
#include "views.hpp"
#include "font.hpp"
#include "../db/account.hpp"

void Arcollect::gui::view_slideshow::set_collection(std::shared_ptr<gui::artwork_collection> &new_collection)
{
	collection = new_collection;
	auto new_collection_iterator = new_collection->begin();
	if (new_collection_iterator != new_collection->end()) {
		// Avoid resetting the artwork to new_collection->begin()
		if (target_artwork) {
			new_collection_iterator = new_collection->find_nearest(target_artwork);
			if (new_collection_iterator == new_collection->end())
				new_collection_iterator = new_collection->begin();
		} else new_collection_iterator = new_collection->begin();
	}
	set_collection_iterator(new_collection_iterator);
}
void Arcollect::gui::view_slideshow::resize(SDL::Rect rect)
{
	this->rect = rect;
	size_know = false;
	if (viewport.artwork) {
		// Preserve aspect ratio
		SDL::Point art_size;
		if (!viewport.artwork->QuerySize(art_size))
			return; // Art is queued for load, wait a little bit.
		size_know = true;
		
		int height_for_width = art_size.y*rect.w/art_size.x;
		if (height_for_width > rect.h) {
			// Perform width for height size
			int width_for_height = art_size.x*rect.h/art_size.y;
			rect.x += (rect.w-width_for_height)/2;
			rect.w = width_for_height;
		} else {
			// Perform height for width size
			rect.y += (rect.h-height_for_width)/2;
			rect.h = height_for_width;
		}
		// Set viewport
		viewport.set_corners(rect);
		// Reset cached title render
		title_text_cache.reset();
	}
}
void Arcollect::gui::view_slideshow::set_collection_iterator(const artwork_collection::iterator &iter)
{
	if (iter != collection->end()) {
		collection_iterator = std::make_unique<artwork_collection::iterator>(iter);
		viewport.artwork = **collection_iterator;
		resize(rect);
	} else viewport.artwork = NULL;
}
void Arcollect::gui::view_slideshow::render_info_incard(void)
{
	Arcollect::db::artwork &artwork = ***collection_iterator;
	// Adjust rect
	const auto box_height = rect.h/5;
	static const auto min_title_font_height = 14;
	static const auto min_desc_font_height = 7;
	auto font_height = box_height/3;
	if (font_height < min_title_font_height)
		font_height = min_title_font_height;
	
	SDL::Rect render_rect{rect.x,rect.h-box_height,rect.w,box_height};
	
	// Draw rect
	renderer->SetDrawColor(0,0,0,192);
	renderer->FillRect(render_rect);
	// Apply padding
	const auto box_padding = rect.h/100;
	render_rect.x += box_padding;
	render_rect.y += box_padding;
	render_rect.w -= 2*box_padding;
	render_rect.h -= 2*box_padding;
	// Render title text
	// TODO Cache this
	Arcollect::gui::font::Renderable title_line(artwork.title().c_str(),font_height,render_rect.w);
	title_line.render_tl(render_rect.x,render_rect.y);
	// Render description text
	// TODO Cache this
	font_height /= 4;
	if (font_height < min_desc_font_height)
		font_height = min_desc_font_height;
	Arcollect::gui::font::Renderable desc_par(artwork.desc().c_str(),font_height,rect.w);
	desc_par.render_tl(render_rect.x,render_rect.y + title_line.size().y + 4);
}

void Arcollect::gui::view_slideshow::render(void)
{
	if (viewport.artwork) {
		// Preload previous artwork
		if (*collection_iterator != collection->begin()) {
			(**--*collection_iterator).query_texture();
			++*collection_iterator;
		}
		// Preload next artwork
		++*collection_iterator;
		if (*collection_iterator != collection->end())
			(***collection_iterator).query_texture();
		--*collection_iterator;
		// Resize is size is unknow
		if (!size_know)
			resize(rect);
		// Render artwork
		if (size_know)
			viewport.render({0,0});
	} else {
		static std::unique_ptr<Arcollect::gui::font::Renderable> no_artwork_text_cache;
		if (!no_artwork_text_cache)
			no_artwork_text_cache = std::make_unique<Arcollect::gui::font::Renderable>("There is no artwork to show",22);
		no_artwork_text_cache->render_tl(rect.x+(rect.w-no_artwork_text_cache->size().x)/2,rect.y+(rect.h-no_artwork_text_cache->size().y)/2);
	}
	//render_info_incard();
}
void Arcollect::gui::view_slideshow::render_titlebar(SDL::Rect target, int window_width)
{
	if (viewport.artwork) {
		// Render artist avatar
		auto accounts = viewport.artwork->get_linked_accounts("account");
		if (accounts.size() > 0) {
			SDL::Rect icon_rect{target.x,target.y,target.h,target.h};
			renderer->Copy(accounts[0]->get_icon().get(),NULL,&icon_rect);
		}
		// Render title
		const int title_border = target.h/4;
		if (!title_text_cache)
			title_text_cache = std::make_unique<font::Renderable>(viewport.artwork->title().c_str(),target.h-2*title_border);
		title_text_cache->render_tl(target.x+title_border+target.h,target.y+title_border);
	}
}
bool Arcollect::gui::view_slideshow::event(SDL::Event &e)
{
	// STOP READING CODE!!! You might not understand some weird syntax.
	// There's a 'README BEFORE READING CODE!!!' in top Arcollect::gui::view_slideshow declaration.
	switch (e.type) {
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_RIGHT: {
					if (viewport.artwork) {
						++*collection_iterator;
						if (*collection_iterator != collection->end()) {
							target_artwork = viewport.artwork = **collection_iterator;
							resize(rect);
						} else --*collection_iterator; // Rewind
					}
				} break;
				case SDL_SCANCODE_LEFT: {
					if (viewport.artwork) {
						if (*collection_iterator != collection->begin()) {
							target_artwork = viewport.artwork = *--*collection_iterator;
							resize(rect);
						}
					}
				} break;
				case SDL_SCANCODE_HOME: {
					if (viewport.artwork) {
						collection_iterator = std::make_unique<gui::artwork_collection::iterator>(collection->begin());
						target_artwork = viewport.artwork = **collection_iterator;
						resize(rect);
					}
				} break;
				case SDL_SCANCODE_END: {
					if (viewport.artwork) {
						collection_iterator = std::make_unique<gui::artwork_collection::iterator>(collection->end());
						target_artwork = viewport.artwork = *--*collection_iterator;
						resize(rect);
					}
				} break;
				default:break;
			}
		} break;
		// Only called for Arcollect::gui::background_slideshow
		case SDL_WINDOWEVENT: {
			switch (e.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				case SDL_WINDOWEVENT_RESIZED: {
					resize({0,0,e.window.data1,e.window.data2});
				} break;
				default: {
				} break;
			}
		} break;
	}
	return false;
}

