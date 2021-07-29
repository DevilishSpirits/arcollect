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
#include <SDL_ttf.h>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
namespace Arcollect {
	namespace gui {
		namespace font {
			TTF_Font *query_font(Uint32 font_size);
			/** Text-element
			 *
			 * A text element can be content (text) or attribute changes (color, ...).
			 *
			 * A list of #Element is used to generate text.
			 */
			typedef std::variant<
				std::string,      // Owned text
				std::string_view // Referenced text
				//SDL_Color,        // Text color change
			> Element;
			enum ElementIndex: std::size_t {
				ELEMENT_STRING,
				ELEMENT_STRING_VIEW,
				//ELEMENT_COLOR,
			};
			/** Vector of #Element
			 *
			 * Used to ease text building with operator<<. Also contain initial state.
			 */
			struct Elements: public std::vector<Element> {
				int       initial_height = 12;
				SDL_Color initial_color = {255,255,255,255};
				
				template <typename T>
				inline Elements& operator<<(const T value) {
					emplace_back(value);
					return *this;
				}
			};
			/** Text shaping result
			 *
			 * This class can render a bloc of text. It can be generated to fit in a
			 * particular box.
			 */
			class Renderable {
				private:
					std::shared_ptr<SDL::Texture> result;
					SDL::Point result_size;
				public:
					inline const SDL::Point size() {
						return result_size;
					}
					/** Renderable empty constructor
					 *
					 * \warning The object is invalid and trying to render is undefined.
					 *          You must set the object to something valid before render.
					 */
					Renderable(void) = default;
					Renderable(const Elements& elements);
					Renderable(const Elements& elements, Uint32 wrap_width);
					/** Convenience simple text rendering
					 *
					 * It use default values of #Elements
					 */
					Renderable(const char* text, int font_size);
					/** Convenience simple text rendering
					 *
					 * It use default values of #Elements
					 */
					Renderable(const char* text, int font_size, Uint32 wrap_width);
					Renderable(const Renderable&) = default;
					void render_tl(int x, int y);
					inline void render_tl(SDL::Point topleft_corner) {
						return render_tl(topleft_corner.x,topleft_corner.y);
					}
			};
		}
	}
}
