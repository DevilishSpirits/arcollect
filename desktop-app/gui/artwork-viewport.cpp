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
	return artwork->render(&rect);
}

void Arcollect::gui::artwork_viewport::set_corners(const SDL::Rect rect)
{
	corner_tl.x = corner_bl.x = rect.x;
	corner_tl.y = corner_tr.y = rect.y;
	corner_tr.x = corner_br.x = rect.x + rect.w;
	corner_bl.y = corner_br.y = rect.y + rect.h;
}
