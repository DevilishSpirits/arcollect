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
#include "animation.hpp"
#include "artwork-viewport.hpp"
#include "artwork-collection.hpp"
#include "font.hpp"
#include "modal.hpp"
#include <list>
#include <vector>
namespace Arcollect {
	namespace gui {
		// Generic view interface
		
		class view: public modal {
			protected:
				std::shared_ptr<gui::artwork_collection> collection;
			public:
				inline std::shared_ptr<gui::artwork_collection> get_collection(void) const {
					return collection;
				}
				/** Set the collection
				 */
				virtual void set_collection(std::shared_ptr<gui::artwork_collection> &new_collection) = 0;
				/** Called upon viewport resize
				 */
				virtual void resize(SDL::Rect rect) = 0;
				virtual ~view(void) = default;
		};
		class view_slideshow: public view {
			/* README BEFORE READING CODE!!!
			 *
			 * `*--*collection_iterator` may seem dark to you :
			 * 	`*collection_iterator` dereference the `std::unique_ptr` getting an (peusdo-)iterator (pseudo because not C++ compliant).
			 * 	`--(*collection_iterator)` perform operator-- on the iterator.
			 * 	`*(--*collection_iterator)` return the pointed std::shared_ptr<artwork>
			 */
			protected:
				// The bounding rect
				SDL::Rect rect;
				// The main artwork viewport
				artwork_viewport viewport;
				bool size_know = false;
				std::unique_ptr<artwork_collection::iterator> collection_iterator;
				std::unique_ptr<font::Renderable> title_text_cache;
			public:
				/** Artwork to target
				 *
				 * When calling set_collection(), try to display the #target_artwork or
				 * the nearest one.
				 *
				 * This make a natural behavior during searchs where the slideshow seem
				 * to "remember" the last artwork when you erase search.
				 */
				std::shared_ptr<db::artwork> target_artwork;
				void set_collection(std::shared_ptr<gui::artwork_collection> &new_collection) override;
				void resize(SDL::Rect rect) override;
				void set_collection_iterator(const artwork_collection::iterator &iter);
				void render(void) override;
				void render_titlebar(SDL::Rect target, int window_width) override;
				/** Render some info in the bottom of the window
				 *
				 * Should be called right after render()
				 *
				 * The card is rendered with a flat transparent black panel in the picture
				 * bottom with white text draw on it.
				 *
				 * The box height is 20% of the rect height. Text height is a third of
				 * this value.
				 *
				 */
				void render_info_incard(void);
				bool event(SDL::Event &e) override;
		};
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
				int artwork_height = 200;
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
				int left_y = 0;
				/** Bottom position of the right (bottom) line
				 *
				 * Used to known when to call new_line_right()
				 */
				int right_y = 0;
				std::unique_ptr<artwork_collection::iterator> left_iter;
				std::unique_ptr<artwork_collection::iterator> right_iter;
				
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
				bool new_line_check_fit(int &free_space, int y, std::vector<artwork_viewport> &new_viewports, artwork_collection::iterator &iter);
				
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
				void set_collection(std::shared_ptr<gui::artwork_collection> &new_collection) override;
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
				void render(void) override;
				void render_viewport_hover(const artwork_viewport& viewport);
				bool event(SDL::Event &e) override;
		};
	}
}
