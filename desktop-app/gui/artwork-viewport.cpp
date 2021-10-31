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
#include "artwork-viewport.hpp"
extern SDL::Renderer *renderer;
int Arcollect::gui::artwork_viewport::render(const SDL::Point displacement)
{
	// Apply displacement
	SDL::Point local_corner_tl = corner_tl + displacement;
	SDL::Point local_corner_tr = corner_tr + displacement;
	SDL::Point local_corner_br = corner_br + displacement;
	SDL::Point local_corner_bl = corner_bl + displacement;
	// TODO Perspective
	// Compute the inner rect
	SDL::Rect rect;
	rect.x = local_corner_tl.x > local_corner_bl.x ? local_corner_tl.x : local_corner_bl.x;
	rect.y = local_corner_tl.y > local_corner_tr.y ? local_corner_tl.y : local_corner_tr.y;
	rect.w = (local_corner_tr.x > local_corner_br.x ? local_corner_tr.x : local_corner_br.x)-rect.x;
	rect.h = (local_corner_bl.y > local_corner_br.y ? local_corner_bl.y : local_corner_br.y)-rect.y;
	// Render
	auto text = download->query_data<std::unique_ptr<SDL::Texture>>();
	if (text) {
		return renderer->Copy(text->get().get(),NULL,&rect);
	} else {
		// Render a placeholder
		renderer->SetDrawColor(0,0,0,192);
		renderer->FillRect(rect);
		SDL::Rect placeholder_rect{rect.x,rect.y,rect.h/4,rect.h/8};
		if (placeholder_rect.w > rect.w*3/4) {
			placeholder_rect.w = rect.w*3/4;
		}
		placeholder_rect.x += (rect.w-placeholder_rect.w)/2;
		placeholder_rect.y += (rect.h-placeholder_rect.h)/2;
		int bar_count;
		const int max_bar_count = 3;
		switch (download->load_state) {
			case Arcollect::db::download::UNLOADED: {
				bar_count = 0;
			} break;
			case Arcollect::db::download::LOAD_SCHEDULED:
			case Arcollect::db::download::LOAD_PENDING_STAGE1: {
				bar_count = 1;
			} break;
			case Arcollect::db::download::LOADING_STAGE1: {
				bar_count = 2;
			} break;
			case Arcollect::db::download::LOAD_PENDING_STAGE2:
			case Arcollect::db::download::LOADED: {
				bar_count = 3;
			} break;
		}
		renderer->SetDrawColor(255,255,255,128);
		renderer->DrawRect(placeholder_rect);
		// Draw inside bars
		auto border = placeholder_rect.w/8;
		placeholder_rect.x += border;
		placeholder_rect.y += border;
		placeholder_rect.h -= border*2;
		placeholder_rect.w -= border*(1+max_bar_count);
		placeholder_rect.w /= max_bar_count;
		const SDL::Color bar_colors[] = {
			{0x000000FF},
			{0x808080FF},
			{0x00ff00FF},
			{0x00ff00FF},
		};
		for (auto i = bar_count; i; i--) {
			const SDL::Color &color = bar_colors[bar_count];
			renderer->SetDrawColor((color.r+(rand()%256))/2,(color.g+(rand()%256))/2,(color.b+(rand()%256))/2,64);
			renderer->FillRect(placeholder_rect);
			renderer->SetDrawColor(color.r,color.g,color.b,128);
			renderer->DrawRect(placeholder_rect);
			placeholder_rect.x += placeholder_rect.w+border;
		}
		return 0;
	}
}

void Arcollect::gui::artwork_viewport::set_corners(const SDL::Rect rect)
{
	corner_tl.x = corner_bl.x = rect.x;
	corner_tl.y = corner_tr.y = rect.y;
	corner_tr.x = corner_br.x = rect.x + rect.w;
	corner_bl.y = corner_br.y = rect.y + rect.h;
}
