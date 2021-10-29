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
#include <math.h>

extern SDL_Window *window;

void Arcollect::gui::view_slideshow::set_collection(std::shared_ptr<artwork_collection> &new_collection)
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
		switch (viewport.artwork->artwork_type) {
			case db::artwork::ARTWORK_TYPE_IMAGE: {
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
				viewport_animation.val_origin = viewport_animation.val_target = rect;
				viewport_delta.x = rect.x;
				viewport_delta.y = rect.y;
				artwork_zoom1.x = rect.w;
				artwork_zoom1.y = rect.h;
				viewport_zoom = rect.w/(float)art_size.x;
			} break;
			case db::artwork::ARTWORK_TYPE_TEXT: {
				text_display.set_static_elements(viewport.artwork->query_font_elements());
			} break;
			case db::artwork::ARTWORK_TYPE_UNKNOWN: {
			} break;
		}
		// Reset cached title render
		title_text_cache.reset();
	}
}
void Arcollect::gui::view_slideshow::update_zoom(void)
{
	SDL::Point art_size;
	if (!viewport.artwork->QuerySize(art_size))
		return;
	SDL::Point border_limits{rect.w/32,rect.h/32};
	// Zoom check
	if ((art_size.x*viewport_zoom < border_limits.x)&&(art_size.y*viewport_zoom < border_limits.y))
		viewport_zoom = std::min(border_limits.x/(float)art_size.x,border_limits.y/(float)art_size.y);
	
	MySDLRect target{viewport_delta.x,viewport_delta.y,static_cast<int>(art_size.x*viewport_zoom),static_cast<int>(art_size.y*viewport_zoom)};
	// Bound check
	if (rect.w - target.w > 2*border_limits.x)
		viewport_delta.x = target.x = rect.x+(rect.w-target.w)/2; // Center
	else if (target.x > border_limits.x)
		viewport_delta.x = target.x = border_limits.x; // Snap left
	else if (target.x < rect.w - target.w - border_limits.x)
		viewport_delta.x = target.x = rect.w - target.w - border_limits.x;
	if (rect.h - target.h > 2*border_limits.y)
		viewport_delta.y = target.y = rect.y+(rect.h-target.h)/2; // Center
	else if (target.y > border_limits.y)
		viewport_delta.y = target.y = border_limits.y; // Snap left
	else if (target.y < rect.h - target.h - border_limits.y)
		viewport_delta.y = target.y = rect.h - target.h - border_limits.y;
	viewport_animation = target;
}
void Arcollect::gui::view_slideshow::zoomat(float delta, SDL::Point point)
{
	// Update zoom
	const double old_zoom = viewport_zoom;
	const double new_zoom = expf(logf(old_zoom)+delta);
	point.x -= viewport_delta.x;
	point.y -= viewport_delta.y;
	viewport_delta.x -= ((point.x/old_zoom)-(point.x/new_zoom))*new_zoom;
	viewport_delta.y -= ((point.y/old_zoom)-(point.y/new_zoom))*new_zoom;
	viewport_zoom = new_zoom;
	
	update_zoom();
}

void Arcollect::gui::view_slideshow::set_collection_iterator(const artwork_collection::iterator &iter)
{
	if (iter != collection->end()) {
		collection_iterator = iter;
		viewport.artwork = db::artwork::query(*collection_iterator);
		resize(rect);
	} else viewport.artwork = NULL;
}
void Arcollect::gui::view_slideshow::render_info_incard(void)
{
	Arcollect::db::artwork &artwork = *db::artwork::query(*collection_iterator);
	// Adjust rect
	const auto box_height = rect.h/5;
	static const auto min_title_font_height = 14;
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
	// Render text
	// TODO Cache this
	Arcollect::gui::font::Elements elements;
	elements << Arcollect::gui::font::FontSize(font_height  ) << artwork.title() << U"\n"s
	         << Arcollect::gui::font::FontSize(font_height/4) << artwork.desc();
	Arcollect::gui::font::Renderable desc_renderable(elements,render_rect.w);
	desc_renderable.render_tl(render_rect.x,render_rect.y);
}
void Arcollect::gui::view_slideshow::render_click_area(const SDL::Rect &target, ClickArea area, ClickState state)
{
	const auto colors = [](ClickState state) -> std::pair<SDL::Color,SDL::Color> {
		switch (state) {
			case CLICK_HOVER:
				return {SDL::Color{128,128,128,128},SDL::Color{255,255,255,255}};
			case CLICK_PRESSED:
				return {SDL::Color{ 16, 16, 16,192},SDL::Color{128,128,128,255}};
			case CLICK_UI_VIEW:
			default:
				return {SDL::Color{  0,  0,  0,128},SDL::Color{255,255,255,128}};
		}
	}(state);
	const SDL::Color &back_color = colors.first;
	const SDL::Color &line_color = colors.second;
	
	const auto border_limit = std::max(rect.w/32,16);
	switch (area) {
		case CLICK_NONE:break;
		case CLICK_PREV: {
			SDL::Rect rect{target.x,target.y + (target.h-border_limit)/2,border_limit,border_limit};
			// Draw backdrop
			renderer->SetDrawColor(back_color);
			renderer->FillRect(rect);
			// Draw arrow
			const SDL::Point arrow{border_limit/8,border_limit/4};
			const auto center = rect.x + rect.w/2;
			const auto middle = rect.y + rect.h/2;
			renderer->SetDrawColor(line_color);
			renderer->DrawLine(center+arrow.x,middle-arrow.y,center-arrow.x,middle);
			renderer->DrawLine(center-arrow.x,middle,center+arrow.x,middle+arrow.y);
		} break;
		case CLICK_NEXT: {
			SDL::Rect rect{target.x + target.w - border_limit,target.y + (target.h-border_limit)/2,border_limit,border_limit};
			// Draw backdrop
			renderer->SetDrawColor(back_color);
			renderer->FillRect(rect);
			// Draw arrow
			const SDL::Point arrow{border_limit/8,border_limit/4};
			const auto center = rect.x + rect.w/2;
			const auto middle = rect.y + rect.h/2;
			renderer->SetDrawColor(line_color);
			renderer->DrawLine(center-arrow.x,middle-arrow.y,center+arrow.x,middle);
			renderer->DrawLine(center+arrow.x,middle,center-arrow.x,middle+arrow.y);
		} break;
	}
}

void Arcollect::gui::view_slideshow::render(SDL::Rect target)
{
	if (viewport.artwork) {
		// Preload previous artwork
		if (collection_iterator != collection->begin()) {
			db::artwork::query(*--collection_iterator)->query_texture();
			++collection_iterator;
		}
		// Preload next artwork
		++collection_iterator;
		if (collection_iterator != collection->end())
			db::artwork::query(*collection_iterator)->query_texture();
		--collection_iterator;
		// Resize is size is unknow
			switch (viewport.artwork->artwork_type) {
				case db::artwork::ARTWORK_TYPE_IMAGE: {
					if (!size_know)
						resize(rect);
					// Render artwork
					if (size_know) {
						viewport.set_corners(viewport_animation);
						viewport.render({0,0});
					}
				} break;
				case db::artwork::ARTWORK_TYPE_TEXT:
					return text_display.render(target);
				case db::artwork::ARTWORK_TYPE_UNKNOWN: {
					Arcollect::gui::font::Elements unknown_artwork_elements;
					unknown_artwork_elements.initial_height() = 22;
					unknown_artwork_elements.initial_color()  = {255,255,0,255};
					unknown_artwork_elements << std::string_view(viewport.artwork->mimetype()) << (SDL::Color){255,255,255,255} << U" artwork type is not supported."sv;
					Arcollect::gui::font::Renderable unknown_artwork_text_cache(unknown_artwork_elements);
					unknown_artwork_text_cache.render_tl(rect.x+(rect.w-unknown_artwork_text_cache.size().x)/2,rect.y+(rect.h-unknown_artwork_text_cache.size().y)/2);
				} break;
			}
		// Render click UI
		SDL::Point mouse_pos;
		auto mouse_clicking = SDL_GetMouseState(&mouse_pos.x,&mouse_pos.y) & SDL_BUTTON(1);
		ClickArea hover_area = click_area(target,mouse_pos);
		ClickState click_state = mouse_clicking && (hover_area == clicking_area)
			? CLICK_PRESSED : CLICK_HOVER;
		render_click_area(target,hover_area,click_state);
	} else {
		static std::unique_ptr<Arcollect::gui::font::Renderable> no_artwork_text_cache;
		if (!no_artwork_text_cache)
			no_artwork_text_cache = std::make_unique<Arcollect::gui::font::Renderable>(Arcollect::gui::font::Elements(U"There is no artwork to show"s,22));
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
		// Render clicks UI
		// TODO Modify the prototype to include this information
		renderer->GetOutputSize(target.w,target.h);
		render_click_area(target,CLICK_PREV,CLICK_UI_VIEW);
		render_click_area(target,CLICK_NEXT,CLICK_UI_VIEW);
	}
}
void Arcollect::gui::view_slideshow::go_first(void)
{
	if (viewport.artwork) {
		collection_iterator = collection->begin();
		target_artwork = viewport.artwork = db::artwork::query(*collection_iterator);
		resize(rect);
	}
}
void Arcollect::gui::view_slideshow::go_prev(void)
{
	if (viewport.artwork) {
		if (collection_iterator != collection->begin()) {
			target_artwork = viewport.artwork = db::artwork::query(*--collection_iterator);
			resize(rect);
		}
	}
}
void Arcollect::gui::view_slideshow::go_next(void)
{
	if (viewport.artwork) {
		++collection_iterator;
		if (collection_iterator != collection->end()) {
			target_artwork = viewport.artwork = db::artwork::query(*collection_iterator);
			resize(rect);
		} else --collection_iterator; // Rewind
	}
}
void Arcollect::gui::view_slideshow::go_last(void)
{
	if (viewport.artwork) {
		collection_iterator = collection->end();
		target_artwork = viewport.artwork = db::artwork::query(*--collection_iterator);
		resize(rect);
	}
}
Arcollect::gui::view_slideshow::ClickArea Arcollect::gui::view_slideshow::click_area(const SDL::Rect &rect, SDL::Point position)
{
	const auto border_limit = rect.w/32;
	position.x -= rect.x;
	position.y -= rect.y;
	if (position.x < border_limit)
		return CLICK_PREV;
	else if (position.x > rect.w - border_limit)
		return CLICK_NEXT;
	return CLICK_NONE;
}
bool Arcollect::gui::view_slideshow::event(SDL::Event &e, SDL::Rect target)
{
	// STOP READING CODE!!! You might not understand some weird syntax.
	// There's a 'README BEFORE READING CODE!!!' in top Arcollect::gui::view_slideshow declaration.
	SDL::Point cursorpos;
	auto mouse_state = SDL_GetMouseState(&cursorpos.x,&cursorpos.y);
	switch (e.type) {
		case SDL_KEYDOWN: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_UP: { // Zoom-in
					if (viewport.artwork)
						switch (viewport.artwork->artwork_type) {
							case ARTWORK_TYPE_UNKNOWN:break;
							case ARTWORK_TYPE_IMAGE: {
								zoomat(+.1f,{rect.x+rect.w/2,rect.y+rect.h/2});
							} break;
							case ARTWORK_TYPE_TEXT:
								return text_display.event(e,target);
						}
				} break;
				case SDL_SCANCODE_DOWN: { // Zoom-out
					if (viewport.artwork)
						switch (viewport.artwork->artwork_type) {
							case ARTWORK_TYPE_UNKNOWN:break;
							case ARTWORK_TYPE_IMAGE: {
								zoomat(-.1f,{rect.x+rect.w/2,rect.y+rect.h/2});
							} break;
							case ARTWORK_TYPE_TEXT:
								return text_display.event(e,target);
						}
				} break;
				default:break;
			}
		} break;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_AC_FORWARD:
				case SDL_SCANCODE_PAGEDOWN:
				case SDL_SCANCODE_RIGHT: {
					go_next();
				} break;
				case SDL_SCANCODE_AC_BACK:
				case SDL_SCANCODE_PAGEUP:
				case SDL_SCANCODE_LEFT: {
					go_prev();
				} break;
				case SDL_SCANCODE_HOME: {
					go_first();
				} break;
				case SDL_SCANCODE_END: {
					go_last();
				} break;
				default:break;
			}
		} break;
		case SDL_MOUSEWHEEL: {
			if (viewport.artwork)
				switch (viewport.artwork->artwork_type) {
					case ARTWORK_TYPE_UNKNOWN:break;
					case ARTWORK_TYPE_IMAGE: {
						SDL::Point cursorpos;
						SDL_GetMouseState(&cursorpos.x,&cursorpos.y);
						zoomat(e.wheel.y*.1f,cursorpos);
					} break;
					case ARTWORK_TYPE_TEXT:
						return text_display.event(e,target);
				}
		} break;
		case SDL_MOUSEMOTION: {
			if ((mouse_state & SDL_BUTTON(1))&&(clicking_area == CLICK_NONE)) {
				viewport_delta.x += e.motion.xrel;
				viewport_delta.y += e.motion.yrel;
				update_zoom();
				// Skip animation
				viewport_animation.val_origin = viewport_animation.val_target;
			}
		} break;
		case SDL_MOUSEBUTTONUP: {
			if (e.button.button & SDL_BUTTON(1)) {
				ClickArea clickup_area = click_area(target,{e.button.x,e.button.y});
				if (clickup_area == clicking_area)
					switch (clickup_area) {
						case CLICK_NONE:break;
						case CLICK_PREV: {
							go_prev();
						} break;
						case CLICK_NEXT: {
							go_next();
						} break;
					}
			}
			clicking_area = CLICK_NONE;
		} break;
		case SDL_MOUSEBUTTONDOWN: {
			if (e.button.button & SDL_BUTTON(1))
				clicking_area = click_area(target,{e.button.x,e.button.y});
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

