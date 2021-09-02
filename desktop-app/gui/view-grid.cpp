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
#include "font.hpp"
#include "views.hpp"
#include "../db/account.hpp"
#include "../db/db.hpp"
void Arcollect::gui::view_vgrid::set_collection(std::shared_ptr<artwork_collection> &new_collection)
{
	collection = new_collection;
	flush_layout();
}

void Arcollect::gui::view_vgrid::flush_layout(void)
{
	if (!collection)
		return;
	// Destroy current layout
	viewports.clear();
	// Reset scrolling
	left_iter = collection->begin();
	right_iter = collection->begin();
	left_y = 0;
	right_y = 0;
	// Force viewport regeneration
	
	do_scroll(0);
	// Inhibate scroll
	scroll_position.val_origin = scroll_position.val_target;
	scroll_position.time_end = 0;
	
	// Invalidate cached caption
	caption_cache_artwork.reset();
}
void Arcollect::gui::view_vgrid::resize(SDL::Rect rect)
{
	this->rect = rect;
	flush_layout();
}
void Arcollect::gui::view_vgrid::render(SDL::Rect target)
{
	// Check if we need to rebuild the layout
	if ((data_version != Arcollect::data_version)|| layout_invalid) {
		data_version = Arcollect::data_version;
		flush_layout();
	}
	// Render
	SDL::Point displacement{0,-scroll_position};
	for (auto &lines: viewports)
		for (artwork_viewport &viewport: lines)
			viewport.render(displacement);
	// Render hover effect on viewport
	SDL::Point cursor_position;
	SDL_GetMouseState(&cursor_position.x,&cursor_position.y);
	artwork_viewport* hover = get_pointed(cursor_position);
	if (hover)
		render_viewport_hover(*hover);
}
void Arcollect::gui::view_vgrid::render_viewport_hover(const artwork_viewport& viewport)
{
	// Draw backdrop
	SDL::Rect rect{viewport.corner_tl.x,viewport.corner_tl.y-scroll_position,viewport.corner_tr.x-viewport.corner_tl.x,viewport.corner_bl.y-viewport.corner_tl.y};
	renderer->SetDrawColor(0,0,0,192);
	renderer->FillRect(rect);
	// Draw title
	if (caption_cache_artwork != viewport.artwork) {
		caption_title = gui::font::Renderable(viewport.artwork->title(),18,rect.w);
		caption_cache_has_artist = false; // Invalidate caption_account
		caption_cache_artwork = viewport.artwork;
	}
	int caption_title_y = rect.y+(rect.h-caption_title.size().y)/2;
	caption_title.render_tl(rect.x+(rect.w-caption_title.size().x)/2,caption_title_y);
	// Draw account
	auto accounts = viewport.artwork->get_linked_accounts("account");
	if (accounts.size()) {
		if (!caption_cache_has_artist) {
			Arcollect::gui::font::Elements elements;
			elements << std::string_view(accounts[0]->title());
			elements.initial_color()  = {255,255,255,192};
			elements.initial_height() = 14;
			caption_account = gui::font::Renderable(elements,rect.w);
			caption_cache_has_artist = true;
		}
		caption_account.render_tl(rect.x+(rect.w-caption_account.size().x)/2,caption_title_y+caption_title.size().y+8);
	}
}
bool Arcollect::gui::view_vgrid::event(SDL::Event &e, SDL::Rect target)
{
	switch (e.type) {
		case SDL_KEYDOWN: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_HOME: {
					do_scroll(-scroll_position);
				} break;
				case SDL_SCANCODE_DOWN: {
					do_scroll(+artwork_height);
				} break;
				case SDL_SCANCODE_PAGEDOWN: {
					do_scroll(+(rect.h/artwork_height)*artwork_height);
				} break;
				case SDL_SCANCODE_UP: {
					do_scroll(-artwork_height);
				} break;
				case SDL_SCANCODE_PAGEUP: {
					do_scroll(-(rect.h/artwork_height)*artwork_height);
				} break;
				default:break;
			}
		} return false;
		case SDL_KEYUP: {
		} return false;
		case SDL_MOUSEWHEEL: {
			do_scroll(-e.wheel.y*artwork_height);
		} return false;
		// Only called Arcollect::gui::background_slideshow
		case SDL_WINDOWEVENT: {
			switch (e.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				case SDL_WINDOWEVENT_RESIZED: {
					resize({0,0,e.window.data1,e.window.data2});
				} break;
				default: {
				} break;
			}
		} return true;
	}
	return true;
}

void Arcollect::gui::view_vgrid::do_scroll(int delta)
{
	auto end_iter = collection->end();
	const auto &scroll_origin = scroll_position.val_origin;
	int scroll_target = scroll_position.val_target + delta;
	
	layout_invalid = false;
	// Create left viewports if needed
	while ((left_y > scroll_target - artwork_height) && !layout_invalid && new_line_left(left_y - artwork_height - artwork_margin.y));
	// Create right viewports if needed
	// NOTE! right_y is offset by minus one row
	while ((right_y < scroll_target + rect.h + artwork_height) && !layout_invalid && (right_iter != end_iter) && new_line_right(right_y));
	// Stop scrolling if bottom is hit
	if (scroll_target + rect.h > right_y)
		scroll_target = right_y - rect.h;
	// Stop scrolling if top is hit
	if (scroll_target < 0)
		scroll_target = 0;
	// Drop left viewports if too much
	while ((left_y < scroll_origin - 2 * artwork_height)&&(left_y < scroll_target - 2 * artwork_height)) {
		left_iter += viewports.front().size();
		viewports.pop_front();
		left_y += artwork_height + artwork_margin.y;
	}
	// Drop right viewports if too much
	while ((right_y > scroll_origin + rect.h + 2 * artwork_height)&&(right_y > scroll_target + rect.h + 2 * artwork_height)) {
		right_iter -= viewports.back().size();
		viewports.pop_back();
		right_y -= artwork_height + artwork_margin.y;
	}
	// Do scrolling
	scroll_position = scroll_target;
}

bool Arcollect::gui::view_vgrid::new_line_left(int y)
{
	const auto begin_iter = collection->begin();
	int free_space = rect.w-2*artwork_margin.x;
	std::vector<artwork_viewport> &new_viewports = viewports.emplace_front();
	// Generate viewports
	while (left_iter != begin_iter) {
		--left_iter;
		if (!new_line_check_fit(free_space,y,new_viewports,left_iter))
			break;
	}
	// Rollback 
	if (left_iter != begin_iter)
		++left_iter;
	// Place viewport horizontally
	if (new_viewports.size()) {
		new_line_place_horizontal_r(free_space,new_viewports);
		left_y -= artwork_height + artwork_margin.x;
		return true;
	} else {
		viewports.pop_front(); // Drop the line
		return false;
	}
}
bool Arcollect::gui::view_vgrid::new_line_right(int y)
{
	auto end_iter = collection->end();
	int free_space = rect.w-2*artwork_margin.x;
	std::vector<artwork_viewport> &new_viewports = viewports.emplace_back();
	// Generate viewports
	while (right_iter != end_iter) {
		if (new_line_check_fit(free_space,y,new_viewports,right_iter))
			++right_iter;
		else break; // Line is full, break
	}
	// Place viewport horizontally
	if (new_viewports.size()) {
		new_line_place_horizontal_l(free_space,new_viewports);
		right_y += artwork_height + artwork_margin.y;
		return true;
	} else {
		viewports.pop_back(); // Drop the line
		return false;
	}
}
bool Arcollect::gui::view_vgrid::new_line_check_fit(int &free_space, int y, std::vector<artwork_viewport> &new_viewports, artwork_collection::iterator &iter)
{
	// Compute width
	SDL::Point size;
	std::shared_ptr<db::artwork> artwork = db::artwork::query(*iter);
	if (!artwork->QuerySize(size)) {
		// Size is unknow, stop for now. Will flush_layout() on next redraw.
		layout_invalid = true;
		return false;
	}
	size.x *= artwork_height;
	size.x /= size.y;
	// Check for overflow
	if (size.x + artwork_margin.x < free_space) {
		// It fit !
		artwork_viewport& viewport = new_viewports.emplace_back();
		viewport.artwork = artwork;
		viewport.iter = std::make_unique<artwork_collection::iterator>(iter);
		viewport.set_corners({artwork_margin.x,y,size.x,artwork_height});
		free_space -= size.x + artwork_margin.x;
	} else {
		// Would overflow
		if (new_viewports.size()) {
			// Rollback and break
			return false;
		} else {
			// This is an ultra large artwork !
			artwork_viewport& viewport = new_viewports.emplace_back();
			viewport.artwork = artwork;
			viewport.iter = std::make_unique<artwork_collection::iterator>(iter);
			viewport.set_corners({artwork_margin.x,y,free_space,artwork_height});
			// Remove all free_space
			free_space = 0;
			return true;
		}
	}
	return true;
}
void Arcollect::gui::view_vgrid::new_line_place_horizontal_l(int free_space, std::vector<artwork_viewport> &new_viewports)
{
	int spacing = artwork_margin.x + free_space / (new_viewports.size() > 1 ? new_viewports.size()-1 : new_viewports.size());
	int x = rect.x;
	for (artwork_viewport& viewport: new_viewports) {
		// Move corners
		viewport.corner_tl.x += x;
		viewport.corner_tr.x += x;
		viewport.corner_br.x += x;
		viewport.corner_bl.x += x;
		// Update x
		x += viewport.corner_tr.x - viewport.corner_tl.x + spacing;
	}
}
void Arcollect::gui::view_vgrid::new_line_place_horizontal_r(int free_space, std::vector<artwork_viewport> &new_viewports)
{
	int spacing = artwork_margin.x + free_space / (new_viewports.size() > 1 ? new_viewports.size()-1 : new_viewports.size());
	int x = rect.x;
	for (auto iter = new_viewports.rbegin(); iter != new_viewports.rend(); ++iter) {
		artwork_viewport& viewport = *iter;
		// Move corners
		viewport.corner_tl.x += x;
		viewport.corner_tr.x += x;
		viewport.corner_br.x += x;
		viewport.corner_bl.x += x;
		// Update x
		x += viewport.corner_tr.x - viewport.corner_tl.x + spacing;
	}
}

Arcollect::gui::artwork_viewport *Arcollect::gui::view_vgrid::get_pointed(SDL::Point mousepos)
{
	mousepos.y -= left_y - scroll_position;
	// Locate the row
	const auto row_height = artwork_height + artwork_margin.y;
	auto pointed_row = mousepos.y / row_height;
	// Check if we are between rows
	if (mousepos.y % row_height > artwork_height)
		return NULL;
	// Check if we are on a row
	if ((pointed_row < 0)||(static_cast<decltype(viewports)::size_type>(pointed_row) >= viewports.size()))
		return NULL;
	// Check viewports
	auto rows_iter = viewports.begin();
	for (;pointed_row--;++rows_iter);
	
	for (auto &viewport: *rows_iter) {
		if ((viewport.corner_tl.x <= mousepos.x)&&(viewport.corner_tr.x >= mousepos.x))
			return &viewport;
	}
	return NULL;
}
