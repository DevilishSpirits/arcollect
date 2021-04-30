#pragma once
#include "../sdl2-hpp/SDL.hpp"
#include "../db/artwork.hpp"
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
			
			/** Render the artwork in the viewport
			 * \param displacement Global displacement added to each corners
			 *
			 * You might want to use SDL clipping.
			 */
			int render(const SDL::Point displacement);
		};
	}
}
