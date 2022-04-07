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
#include "font.hpp"
#include "scrolling-text.hpp"
#include "view.hpp"
namespace Arcollect {
	namespace gui {
		class view_slideshow: public view {
			protected:
				db::artwork::File displayed_file = db::artwork::FILE_ARTWORK;
				// The bounding rect
				SDL::Rect rect;
				/** Size of the artwork at default zoom (100%)
				 */
				SDL::Point artwork_zoom1;
				
				// ARTWORK_TYPE_IMAGE stuff
				artwork_viewport viewport;
				/** Special SDL::Rect for animation purpose
				 *
				 * This SDL::Rect overload operator in order to be animatable by the
				 * #Arcollect::gui::animation::scrolling template.
				 */
				struct MySDLRect: public SDL::Rect {
					MySDLRect operator+(const MySDLRect& other) {
						return {x+other.x,y+other.y,w+other.w,h+other.h};
					}
					MySDLRect operator-(const MySDLRect& other) {
						return {x-other.x,y-other.y,w-other.w,h-other.h};
					}
					MySDLRect operator*(const float other) {
						return {static_cast<int>(x*other),static_cast<int>(y*other),static_cast<int>(w*other),static_cast<int>(h*other)};
					}
					MySDLRect &operator=(const SDL::Rect& other) {
						static_cast<SDL::Rect&>(*this) = other;
						return *this;
					}
				};
				animation::scrolling<MySDLRect> viewport_animation;
				float viewport_zoom;
				SDL::Point viewport_delta;
				void update_zoom(void);
				void zoomat(float delta, SDL::Point point);
				
				// ARTWORK_TYPE_TEXT stuff
				scrolling_text text_display;
				
				bool size_know = false;
				db::artwork_collection::iterator collection_iterator;
				std::unique_ptr<font::Renderable> title_text_cache;
				
				enum ClickState {
					/** Button is hovered
					 */
					CLICK_HOVER,
					/** Button is pressed
					 */
					CLICK_PRESSED,
					/** Topbar is hovered and the UI is displayed
					 */
					CLICK_UI_VIEW,
				};
				enum ClickArea {
					/** Do nothing
					 */
					CLICK_NONE,
					/** Go to the previous artwork
					 *
					 * Call go_prev() on click.
					 */
					CLICK_PREV,
					/** Go to the next artwork
					 *
					 * Call go_next() on click.
					 */
					CLICK_NEXT,
				};
				ClickArea clicking_area = CLICK_NONE;
				/** Get the action to do on click
				 * \param rect     The rendering rect
				 * \param position The mouse position
				 * \return The action to do on click
				 */
				ClickArea click_area(const SDL::Rect &rect, SDL::Point position);
				/** Render the click area UI
				 * \param area The area to render
				 */
				void render_click_area(const Arcollect::gui::modal::render_context &render_ctx, ClickArea area, ClickState state);
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
				void set_collection(std::shared_ptr<artwork_collection> &new_collection) override;
				void resize(SDL::Rect rect); // TODO Only resize upon width/height change
				void set_collection_iterator(const artwork_collection::iterator &iter);
				void render(Arcollect::gui::modal::render_context render_ctx) override;
				void render_titlebar(Arcollect::gui::modal::render_context render_ctx) override;
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
				void render_info_incard(const Arcollect::gui::modal::render_context &render_ctx);
				bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override;
				
				/** Go to the first artwork
				 *
				 * Aka the home button
				 */
				void go_first(void);
				/** Go to the previous artwork
				 *
				 * Aka the left arrow
				 */
				void go_prev(void);
				/** Go to the next artwork
				 *
				 * Aka the right arrow
				 */
				void go_next(void);
				/** Go to the last artwork
				 *
				 * Aka the end button
				 */
				void go_last(void);
		};
	}
}
