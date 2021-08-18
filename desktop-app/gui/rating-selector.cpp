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
#include "rating-selector.hpp"
#include "../db/filter.hpp"

extern SDL::Renderer* renderer;

SDL::Rect Arcollect::gui::rating_selector::rect_tool(SDL::Rect &rect)
{
	// C++ guarantee that these bool are 1 is set and zero if not set
	auto real_width = (has_kid+has_mature+has_adult)*rect.h;
	rect.x += rect.w - real_width;
	rect.w  = real_width;
	return {rect.x,rect.y,rect.h,rect.h};
}
Arcollect::config::Rating Arcollect::gui::rating_selector::pointed_rating(SDL::Rect target, SDL::Point cursor)
{
	// Check for if mouse is outside the cell for all sides but left
	if ((cursor.y < target.y)||(cursor.y > target.y+target.h)||(cursor.x > target.x+target.w))
		return static_cast<Arcollect::config::Rating>(-1);
	// Check if cursor is outside left
	rect_tool(target);
	if (cursor.x < target.x)
		return static_cast<Arcollect::config::Rating>(-1);
	// We are on an item, detect which one
	cursor.x -= target.h;
	if (has_kid) {
		if (cursor.x < target.x)
			return Arcollect::config::Rating::RATING_NONE;
		cursor.x -= target.h;
	}
	if (has_mature) {
		if (cursor.x < target.x)
			return Arcollect::config::Rating::RATING_MATURE;
		cursor.x -= target.h;
	}
	if (has_adult) {
		if (cursor.x < target.x)
		return Arcollect::config::Rating::RATING_ADULT;
	}
	// Reaching this code path should be impossible
	return static_cast<Arcollect::config::Rating>(-1);
}
void Arcollect::gui::rating_selector::render(SDL::Rect target)
{
	renderer->SetDrawBlendMode(SDL::BLENDMODE_BLEND);
	// Compute bound
	SDL::Rect cell = rect_tool(target);
	int inner_border = cell.h/4;
	SDL::Rect selected_cell{0,0,0,0};
	SDL::Rect hover_cell{0,0,0,0};
	// Draw cells
	bool draw_left_border = true;
	if (has_kid) {
		// Draw background
		renderer->SetDrawColor(0,128,0,128);
		renderer->FillRect(cell);
		renderer->SetDrawColor(0,255,0,255);
		// Draw borders
		renderer->DrawLine(cell.x,cell.y,cell.x+cell.w,cell.y);
		renderer->DrawLine(cell.x,cell.y+cell.h,cell.x+cell.w,cell.y+cell.h);
		if (draw_left_border) {
			renderer->DrawLine(cell.x,cell.y,cell.x,cell.y+cell.h);
			draw_left_border = false;
		}
		
		if (rating == Arcollect::config::Rating::RATING_NONE)
			selected_cell = cell;
		if (hover_rating == Arcollect::config::Rating::RATING_NONE)
			hover_cell = cell;
		
		cell.x += cell.w;
	}
	if (has_mature) {
		// Draw background
		renderer->SetDrawColor(0,0,128,128);
		renderer->FillRect(cell);
		// Draw a M
		renderer->SetDrawColor(0,128,255,255);
		renderer->DrawLine(cell.x+inner_border,cell.y+cell.h-inner_border,cell.x+inner_border,cell.y+inner_border);
		renderer->DrawLine(cell.x+inner_border,cell.y+inner_border,cell.x+cell.w/2,cell.y+cell.h/2);
		renderer->DrawLine(cell.x+cell.w/2,cell.y+cell.h/2,cell.x+cell.w-inner_border,cell.y+inner_border);
		renderer->DrawLine(cell.x+cell.w-inner_border,cell.y+cell.h-inner_border,cell.x+cell.w-inner_border,cell.y+inner_border);
		// Draw borders
		renderer->DrawLine(cell.x,cell.y,cell.x+cell.w,cell.y);
		renderer->DrawLine(cell.x,cell.y+cell.h,cell.x+cell.w,cell.y+cell.h);
		if (draw_left_border) {
			renderer->DrawLine(cell.x,cell.y,cell.x,cell.y+cell.h);
			draw_left_border = false;
		}
		
		if (rating == Arcollect::config::Rating::RATING_MATURE)
			selected_cell = cell;
		if (hover_rating == Arcollect::config::Rating::RATING_MATURE)
			hover_cell = cell;
		
		cell.x += cell.w;
	}
	if (has_adult) {
		// Draw background
		renderer->SetDrawColor(128,0,0,128);
		renderer->FillRect(cell);
		// Draw a cross
		renderer->SetDrawColor(255,0,0,255);
		renderer->DrawLine(cell.x+inner_border,cell.y+inner_border,cell.x+cell.w-inner_border,cell.y+cell.h-inner_border);
		renderer->DrawLine(cell.x+cell.w-inner_border,cell.y+inner_border,cell.x+inner_border,cell.y+cell.h-inner_border);
		// Draw borders
		renderer->DrawLine(cell.x,cell.y,cell.x+cell.w,cell.y);
		renderer->DrawLine(cell.x,cell.y+cell.h,cell.x+cell.w,cell.y+cell.h);
		if (draw_left_border) {
			renderer->DrawLine(cell.x,cell.y,cell.x,cell.y+cell.h);
			draw_left_border = false;
		}
		
		if (rating == Arcollect::config::Rating::RATING_ADULT)
			selected_cell = cell;
		if (hover_rating == Arcollect::config::Rating::RATING_ADULT)
			hover_cell = cell;
		
		cell.x += cell.w;
	}
	// Draw right border
	// Note that the draw color is already set to the right cell since this is the last drawn
	renderer->DrawLine(cell.x,cell.y,cell.x,cell.y+cell.h);
	
	// Draw the hover cell
	if (hover_cell.w) {
		renderer->SetDrawColor(255,255,255,64);
		renderer->FillRect(hover_cell);
	}
	// Draw the selected_cell
	if (selected_cell.w) {
		selected_cell.x -= 2;
		selected_cell.y -= 2;
		selected_cell.w += 4;
		selected_cell.h += 5;
		renderer->SetDrawColor(255,255,255,192);
		renderer->DrawRect(selected_cell);
	}
}
void Arcollect::gui::rating_selector::event(SDL::Event &e, SDL::Rect target)
{
	switch (e.type) {
		// FIXME Change should happen after releasing
		case SDL_MOUSEMOTION: {
			hover_rating = pointed_rating(target,{e.motion.x,e.motion.y});
		} break;
		case SDL_MOUSEBUTTONDOWN: {
			auto new_rating = pointed_rating(target,{e.button.x,e.button.y});
			if (new_rating != static_cast<Arcollect::config::Rating>(-1)) {
				rating = new_rating;
				onratingset(new_rating);
			}
		} break;
		default: break;
	}
}

static void set_rating(Arcollect::config::Rating new_rating)
{
	Arcollect::db_filter::set_rating(new_rating);
}
Arcollect::gui::rating_selector_menu::rating_selector_menu(void) :
	text_line(font::Elements(U"Rating"s,14))
{
	selector.has_kid = true;
	selector.has_mature = true;
	selector.has_adult = true;
	selector.onratingset = set_rating;
}
SDL::Point Arcollect::gui::rating_selector_menu::size(void)
{
	SDL::Point size = text_line.size();
	size.x += (1+selector.has_kid+selector.has_mature+selector.has_adult)*size.y;
	return size;
}
void Arcollect::gui::rating_selector_menu::event(SDL::Event &e, const SDL::Rect &event_location, const SDL::Rect &render_location)
{
	selector.event(e,render_location);
}
void Arcollect::gui::rating_selector_menu::render(SDL::Rect target)
{
	// Update rating
	selector.rating = static_cast<Arcollect::config::Rating>(static_cast<int>(Arcollect::config::current_rating));
	// Render label
	text_line.render_tl(target.x,target.y+(target.h-text_line.size().y)/2);
	//Render selector
	selector.render(target);
}
