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
#include "view.hpp"
#include "animation.hpp"
#include "artwork-viewport.hpp"
#include "font.hpp"
#include <list>
#include <vector>
namespace Arcollect {
	namespace gui {
		/** Vertical grid view
		 *
		 * This view show artworks in a grid layout.
		 *
		 * It is capable of smart infinite scrolling by only rendering displayed
		 * artworks with a scolling window.
		 *
		 * The class use left and right words, left is about previous artworks while
		 * right is about next artworks because we keep a pair of iterators at those
		 * positions and play with.
		 */
		class view_vgrid: public view {
			private:
				db::artwork::File displayed_file = db::artwork::FILE_THUMBNAIL;
				/** The data version
				 *
				 * Used to flush_layout() if the content change
				 */
				sqlite_int64 data_version;
				// The bounding rect
				SDL::Rect rect;
				
				// Infos caption caches
				std::shared_ptr<db::artwork> caption_cache_artwork;
				bool caption_cache_has_artist; // To cope with DB change
				gui::font::Renderable caption_title;
				gui::font::Renderable caption_account;
				
				/** Layout invalid flag
				 *
				 * If set, this mean that the layout must be recomputed upon next draw.
				 */
				bool layout_invalid;
				/** Artworks height in pixel
				 */
				int artwork_height;
				/** Artworks margins in pixel
				 */
				SDL::Point artwork_margin = {10,10};
				/* Scroll position from the top
				 */
				animation::scrolling<int> scroll_position = 0;
				/** Perform scrolling
				 */
				void do_scroll(int delta);
				/** Top position of the left (top) line
				 *
				 * Used to known when to call new_line_left()
				 */
				int left_y = artwork_margin.y;
				/** Bottom position of the right (bottom) line
				 *
				 * Used to known when to call new_line_right()
				 */
				int right_y = artwork_margin.y;
				db::artwork_collection::iterator left_iter;
				db::artwork_collection::iterator right_iter;
				
				/** Viewports array
				 *
				 * This is a list of viewports rows.
				 */
				std::list<std::vector<artwork_viewport>> viewports;
				
				/** new_line_left()/new_line_right() helper to place viewport horizontally
				 * \param free_space The remaining space.available
				 *
				 * This version place viewports from the left
				 */
				void new_line_place_horizontal_l(int free_space, std::vector<artwork_viewport> &new_viewports);
				/** new_line_left()/new_line_right() helper to place viewport horizontally
				 * \param free_space The remaining space.available
				 *
				 * This version place viewports from the right
				 */
				void new_line_place_horizontal_r(int free_space, std::vector<artwork_viewport> &new_viewports);
				/** new_line_left()/new_line_right() helper to check if viewport fit
				 * \param free_space The remaining space.available
				 * \param iter       The iterator yielding the artwork
				 *
				 * This function also append the artwork if it does fit
				 */
				bool new_line_check_fit(int &free_space, int y, std::vector<artwork_viewport> &new_viewports, db::artwork_collection::iterator &iter);
				
				/** Create a new line in the top
				 * \param y Distance from the logical top
				 *
				 * This function generate a new line of artworks. It edit left_iter.
				 */
				bool new_line_left(int y);
				/** Create a new line
				 * \param y Distance from the logical top
				 */
				bool new_line_right(int y);
			public:
				void set_collection(std::shared_ptr<artwork_collection> &new_collection) override;
				void resize(SDL::Rect rect) override;
				/** Flush and rebuild viewports
				 *
				 * This function destroy all viewports and reset iterators.
				 *
				 * It is called upon some change when cached states in #viewports is no
				 * longer valid.
				 */
				void flush_layout(void);
				/** Get the pointer artwork viewport
				 * \param mousepos The mouse position.
				 */
				artwork_viewport *get_pointed(SDL::Point mousepos);
				void render(Arcollect::gui::modal::render_context render_ctx) override;
				void render_viewport_hover(const artwork_viewport& viewport);
				bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override;
		};
	}
}
