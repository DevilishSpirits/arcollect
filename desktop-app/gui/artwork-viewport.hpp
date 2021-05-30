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
#pragma once
#include "../sdl2-hpp/SDL.hpp"
#include "../db/artwork.hpp"
#include "artwork-collection.hpp"
namespace Arcollect {
	namespace gui {
		/** View for an artwork
		 *
		 * This class implement moving viewport displaying artworks.
		 * Those viewport are animated with smooth movements.
		 *
		 * All arts are displayed using those viewports. They are fully automated
		 * and easy to manipulate.
		 *
		 * \todo While the interface allow perspective, this is currently not
		 *       supported. The picture is drawn as a rectangle fitting the area.
		 */
		struct artwork_viewport {
			SDL::Point corner_tl;
			SDL::Point corner_tr;
			SDL::Point corner_br;
			SDL::Point corner_bl;
			void set_corners(const SDL::Rect rect);
			
			std::shared_ptr<Arcollect::db::artwork> artwork;
			/** User iterator
			 *
			 * This is an optionnal iterator for user code.
			 */
			std::unique_ptr<artwork_collection::iterator> iter;
			
			/** Render the artwork in the viewport
			 * \param displacement Global displacement added to each corners
			 *
			 * You might want to use SDL clipping.
			 */
			int render(const SDL::Point displacement);
		};
	}
}
