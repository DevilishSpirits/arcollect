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
	collection_iterator = std::make_unique<gui::artwork_collection::iterator>(new_collection->begin());
	collection = new_collection;
	if (*collection_iterator != new_collection->end())
		viewport.artwork = **collection_iterator;
}
void Arcollect::gui::view_slideshow::resize(SDL::Rect rect)
{
	this->rect = rect;
	if (viewport.artwork) {
		// Preserve aspect ratio
		SDL::Point art_size;
		viewport.artwork->QuerySize(art_size);
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
	}
}
void Arcollect::gui::view_slideshow::set_collection_iterator(artwork_collection::iterator &iter)
{
	collection_iterator = std::make_unique<artwork_collection::iterator>(iter);
	viewport.artwork = **collection_iterator;
	resize(rect);
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
	renderer->SetDrawBlendMode(SDL::BLENDMODE_BLEND);
	renderer->FillRect(render_rect);
	// Apply padding
	const auto box_padding = rect.h/100;
	render_rect.x += box_padding;
	render_rect.y += box_padding;
	render_rect.w -= 2*box_padding;
	render_rect.h -= 2*box_padding;
	// Render title text
	Arcollect::gui::Font font;
	Arcollect::gui::TextLine title_line(font,artwork.title(),font_height);
	std::unique_ptr<SDL::Texture> title_line_text(title_line.render());
	SDL::Point title_line_size;
	title_line_text->QuerySize(title_line_size);
	// NOTE The text overload
	render_rect.w = title_line_size.x;
	render_rect.h = title_line_size.y;
	renderer->Copy(title_line_text.get(),NULL,&render_rect);
	// Render description text
	font_height /= 4;
	if (font_height < min_desc_font_height)
		font_height = min_desc_font_height;
	Arcollect::gui::TextPar desc_par(font,artwork.desc(),min_title_font_height);
	std::unique_ptr<SDL::Texture> desc_par_text(desc_par.render(rect.w-2*box_padding));
	SDL::Point desc_par_size;
	desc_par_text->QuerySize(desc_par_size);
	render_rect.y += render_rect.h + 4;
	render_rect.w += render_rect.h + 4;
	render_rect.w = desc_par_size.x;
	render_rect.h = desc_par_size.y;
	renderer->Copy(desc_par_text.get(),NULL,&render_rect);
}

void Arcollect::gui::view_slideshow::render(void)
{
	renderer->SetDrawBlendMode(SDL::BLENDMODE_NONE);
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
		// Render artwork
		viewport.render({0,0});
	} else {
		// FIXME 
		Arcollect::gui::Font font;
		Arcollect::gui::TextLine title_line(font,"There is no artwork to show",22);
		std::unique_ptr<SDL::Texture> title_line_text(title_line.render());
		SDL::Point title_line_size;
		title_line_text->QuerySize(title_line_size);
		SDL::Rect render_rect{rect.x+(rect.w-title_line_size.x)/2,rect.y+(rect.h-title_line_size.y)/2,title_line_size.x,title_line_size.y};
		renderer->Copy(title_line_text.get(),NULL,&render_rect);
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
		Arcollect::gui::Font font;
		Arcollect::gui::TextLine title_line(font,viewport.artwork->title(),target.h-2*title_border);
		std::unique_ptr<SDL::Texture> title_line_text(title_line.render());
		SDL::Point title_line_size;
		title_line_text->QuerySize(title_line_size);
		SDL::Rect title_line_dstrect{target.x+title_border+target.h,target.y+title_border,title_line_size.x,title_line_size.y};
		renderer->Copy(title_line_text.get(),NULL,&title_line_dstrect);
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
							viewport.artwork = **collection_iterator;
							resize(rect);
						} else --*collection_iterator; // Rewind
					}
				} break;
				case SDL_SCANCODE_LEFT: {
					if (viewport.artwork) {
						if (*collection_iterator != collection->begin()) {
							viewport.artwork = *--*collection_iterator;
							resize(rect);
						}
					}
				} break;
				case SDL_SCANCODE_HOME: {
					if (viewport.artwork) {
						collection_iterator = std::make_unique<gui::artwork_collection::iterator>(collection->begin());
						viewport.artwork = **collection_iterator;
						resize(rect);
					}
				} break;
				case SDL_SCANCODE_END: {
					if (viewport.artwork) {
						collection_iterator = std::make_unique<gui::artwork_collection::iterator>(collection->end());
						viewport.artwork = *--*collection_iterator;
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

