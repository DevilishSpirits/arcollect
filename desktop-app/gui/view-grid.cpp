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
#include "view-vgrid.hpp"
#include "../config.hpp"
#include "../db/account.hpp"
#include "../db/db.hpp"
#include "../db/sorting.hpp"
void Arcollect::gui::view_vgrid::set_collection(std::shared_ptr<artwork_collection> &new_collection)
{
	collection = new_collection;
	flush_layout();
}

void Arcollect::gui::view_vgrid::check_layout(const Arcollect::gui::modal::render_context &render_ctx)
{
	if (layout_invalid
	||(data_version != Arcollect::data_version) // Check for data version
	||(render_ctx.target.w != last_render_size.x) // Check for render width change
	||(render_ctx.target.h != last_render_size.y) // Check for render height change
	) {
		// Update invalidation detectors
		data_version = Arcollect::data_version;
		last_render_size.x = render_ctx.target.w;
		last_render_size.y = render_ctx.target.h;
		// Flush layout
		flush_layout();
	}
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
	left_y = artwork_margin.y;
	right_y = artwork_margin.y;
	// Update artwork_height
	SDL_Rect screen0_rect;
	if (!SDL_GetDisplayBounds(0,&screen0_rect))
		artwork_height = (screen0_rect.h - artwork_margin.y)/Arcollect::config::rows_per_screen - artwork_margin.y;
	
	// Force viewport regeneration
	
	do_scroll(0);
	// Inhibate scroll
	scroll_position.val_origin = scroll_position.val_target;
	scroll_position.time_end = 0;
	
	// Invalidate cached caption
	caption_cache_artwork.reset();
}
void Arcollect::gui::view_vgrid::render(Arcollect::gui::modal::render_context render_ctx)
{
	check_layout(render_ctx);
	// Drop left viewports if too much
	while ((left_y < scroll_position.val_origin - 2 * artwork_height)&&(left_y < scroll_position.val_target - 2 * artwork_height)) {
		left_iter += viewports.front().size();
		viewports.pop_front();
		left_y += artwork_height + artwork_margin.y;
	}
	// Drop right viewports if too much
	while ((right_y > scroll_position.val_origin + last_render_size.y + 2 * artwork_height)&&(right_y > scroll_position.val_target + last_render_size.y + 2 * artwork_height)) {
		right_iter -= viewports.back().size();
		viewports.pop_back();
		right_y -= artwork_height + artwork_margin.y;
	}
	// Render
	SDL::Point displacement{render_ctx.target.x,render_ctx.target.y-scroll_position};
	for (auto &lines: viewports)
		for (artwork_viewport &viewport: lines)
			viewport.render(displacement);
	// Render hover effect on viewport
	SDL::Point cursor_position;
	SDL_GetMouseState(&cursor_position.x,&cursor_position.y);
	artwork_viewport* hover = get_pointed(render_ctx,cursor_position);
	if (hover)
		render_viewport_hover(render_ctx,*hover,displacement);
}
void Arcollect::gui::view_vgrid::render_viewport_hover(const Arcollect::gui::modal::render_context &render_ctx, const artwork_viewport& viewport, SDL::Point offset)
{
	// Draw backdrop
	SDL::Rect rect{viewport.corner_tl.x+offset.x,viewport.corner_tl.y+offset.y,viewport.corner_tr.x-viewport.corner_tl.x,viewport.corner_bl.y-viewport.corner_tl.y};
	render_ctx.renderer.SetDrawColor(0,0,0,192);
	render_ctx.renderer.FillRect(rect);
	// Draw title
	if (caption_cache_artwork != viewport.artwork) {
		caption_title = gui::font::Renderable(font::Elements::build(viewport.artwork->title()),rect.w);
		caption_cache_has_artist = false; // Invalidate caption_account
		caption_cache_artwork = viewport.artwork;
	}
	int caption_title_y = rect.y+(rect.h-caption_title.size().y)/2;
	caption_title.render_tl(rect.x+(rect.w-caption_title.size().x)/2,caption_title_y);
	// Draw account
	auto accounts = viewport.artwork->get_linked_accounts("account");
	if (accounts.size()) {
		if (!caption_cache_has_artist) {
			auto elements = Arcollect::gui::font::Elements::build(SDL::Color{255,255,255,192},std::string_view(accounts[0]->title()));
			caption_account = gui::font::Renderable(elements,rect.w);
			caption_cache_has_artist = true;
		}
		caption_account.render_tl(rect.x+(rect.w-caption_account.size().x)/2,caption_title_y+caption_title.size().y+8);
	}
}
bool Arcollect::gui::view_vgrid::event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx)
{
	check_layout(render_ctx);
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
					do_scroll(+(render_ctx.target.h/artwork_height)*artwork_height);
				} break;
				case SDL_SCANCODE_UP: {
					do_scroll(-artwork_height);
				} break;
				case SDL_SCANCODE_PAGEUP: {
					do_scroll(-(render_ctx.target.h/artwork_height)*artwork_height);
				} break;
				default:break;
			}
		} return false;
		case SDL_KEYUP: {
		} return false;
		case SDL_MOUSEWHEEL: {
			do_scroll(-e.wheel.y*artwork_height);
		} return false;
	}
	return true;
}

void Arcollect::gui::view_vgrid::do_scroll(int delta)
{
	auto end_iter = collection->end();
	int scroll_target = scroll_position.val_target + delta;
	
	layout_invalid = false;
	// Create left viewports if needed
	while ((left_y > scroll_target - artwork_height) && new_line_left());
	// Create right viewports if needed
	// NOTE! right_y is offset by minus one row
	while ((right_y < scroll_target + last_render_size.y + artwork_height) && (right_iter != end_iter) && new_line_right());
	// Stop scrolling if bottom is hit
	if (scroll_target + last_render_size.y > right_y)
		scroll_target = right_y - last_render_size.y;
	// Stop scrolling if top is hit
	if (scroll_target < 0)
		scroll_target = 0;
	// Do scrolling
	scroll_position = scroll_target;
}

bool Arcollect::gui::view_vgrid::new_line_left(void)
{
	const int y = left_y - artwork_height - artwork_margin.y;
	const auto begin_iter = collection->begin();
	int free_space = last_render_size.x-2*artwork_margin.x;
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
bool Arcollect::gui::view_vgrid::new_line_right(void)
{
	auto end_iter = collection->end();
	int free_space = last_render_size.x-2*artwork_margin.x;
	std::vector<artwork_viewport> &new_viewports = viewports.emplace_back();
	// Generate viewports
	while (right_iter != end_iter) {
		if (new_line_check_fit(free_space,right_y,new_viewports,right_iter))
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
	std::shared_ptr<db::download> download = artwork->get(displayed_file);
	if (!download->QuerySize(size)) {
		// Size is unknow, skip. Will flush_layout() on next redraw.
		layout_invalid = true;
		return true;
	}
	size.x *= artwork_height;
	size.x /= size.y;
	// Check for overflow
	if (size.x + artwork_margin.x < free_space) {
		// It fit !
		artwork_viewport& viewport = new_viewports.emplace_back();
		viewport.set_artwork(artwork,displayed_file);
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
			viewport.set_artwork(artwork,displayed_file);
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
	int x = 0;
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
	int x = 0;
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

Arcollect::gui::artwork_viewport *Arcollect::gui::view_vgrid::get_pointed(const Arcollect::gui::modal::render_context &render_ctx, SDL::Point mousepos)
{
	mousepos.x -= render_ctx.target.x;
	mousepos.y -= left_y - scroll_position + render_ctx.target.y;
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

void Arcollect::gui::view_vgrid::bring_to_view(const Arcollect::gui::modal::render_context &render_ctx, const std::shared_ptr<Arcollect::db::artwork> &artwork)
{
	check_layout(render_ctx);
	auto& compare = Arcollect::db::sorting(collection->sorting_type).compare_arts;
	// Generate lines until we find the target
	if (left_iter != collection->end())
		while (compare(*artwork,*Arcollect::db::artwork::query(*left_iter)) && new_line_left());
	if (right_iter != collection->end())
		while (compare(*Arcollect::db::artwork::query(*right_iter),*artwork) && new_line_right());
	// Search for the artwork
	for (const auto& row: viewports) {
		if (!(compare(*row.front().artwork,*artwork) && compare(*row.back().artwork,*artwork))) {
			// We found the line where it should be
			const auto top_y = row.front().corner_tl.y;
			const auto bot_y = row.front().corner_bl.y;
			// Check if the line is out of sight
			if ((top_y >= scroll_position.val_target+render_ctx.target.h)||(bot_y <= scroll_position.val_target))
				// Scroll to the center
				do_scroll(top_y+((bot_y-top_y)/2)-(render_ctx.target.h/2)-scroll_position.val_target);
			break;
		}
	}
}
