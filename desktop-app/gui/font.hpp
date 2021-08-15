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
#include <hb.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
namespace Arcollect {
	namespace gui {
		namespace font {
			struct FontSize {
				int size;
				inline operator decltype(size)&(void) {return size;}
				inline operator const decltype(size)&(void) const {return size;}
				inline FontSize& operator=(decltype(size) new_size) {
					size = new_size;
					return *this;
				}
				inline decltype(size) operator=(const FontSize& new_size) {
					size = new_size.size;
					return *this;
				}
				inline FontSize(decltype(size) size) : size(size) {}
				FontSize(const FontSize&) = default;
				static const FontSize normal;
			};
			/** Arcollect FreeType2 render flags
			 */
			static constexpr const auto ft_flags = 0;
			
			FT_Face query_face(Uint32 font_size);
			/** Text-element
			 *
			 * A text element can be content (text) or attribute changes (color, ...).
			 *
			 * A list of #Element is used to generate text.
			 */
			typedef std::variant<
				std::string,      // Owned text
				std::string_view, // Referenced text
				FontSize,         // Font size
				SDL_Color        // Text color change
			> Element;
			enum ElementIndex: std::size_t {
				ELEMENT_STRING,
				ELEMENT_STRING_VIEW,
				ELEMENT_FONT_SIZE,
				ELEMENT_COLOR,
			};
			/** Vector of #Element
			 *
			 * Used to ease text building with operator<<. Also contain initial state.
			 */
			struct Elements: public std::vector<Element> {
				FontSize  initial_height = FontSize::normal;
				SDL_Color initial_color = {255,255,255,255};
				
				template <typename T>
				inline Elements& operator<<(const T value) {
					emplace_back(std::move(value));
					return *this;
				}
				// Allow stealing of std::string trough std::move constructs
				inline Elements& operator<<(std::string &&value) {
					emplace_back(value);
					return *this;
				}
			};
			
			/** Cached glyph
			 *
			 * This structure hold the renderered version of a character Glyph.
			 *
			 * \todo The current implementation is using SDL_RenderDrawPoint to "copy"
			 *       the FT_Bitmap. Drop the bitmap in a texture instead.
			 */
			struct Glyph {
				SDL::Texture* text;
				SDL::Rect coordinates;
				
				struct key {
					hb_codepoint_t glyphid;
					int font_size;
					struct hash {
						inline std::size_t operator()(const key& key) const {
							return (std::hash<hb_codepoint_t>()(key.glyphid) << 1) ^ std::hash<int>()(key.font_size);
						}
					};
					inline bool operator==(const key& other) const {
						return (glyphid == other.glyphid)&&(font_size == other.font_size);
					}
				};
				
				Glyph(hb_codepoint_t glyphid, int font_size);
				inline Glyph(const key& key) : Glyph(key.glyphid,key.font_size) {}
				~Glyph(void);
				
				void render(int origin_x, int origin_y, SDL_Color color) const;
				inline void render(SDL::Point origin, SDL_Color color) const {
					return render(origin.x,origin.y,color);
				}
				// Glyph cache
				static std::unordered_map<key,std::unique_ptr<Glyph>,key::hash> glyph_cache;
				static inline Glyph& query(const key& key) {
					auto iter = glyph_cache.find(key);
					if (iter == glyph_cache.end())
						iter = glyph_cache.emplace(key,std::make_unique<Glyph>(key)).first;
					return *(iter->second);
				}
				static inline Glyph& query(hb_codepoint_t glyphid, int font_size) {
					return query(key{glyphid,font_size});
				}
			};
			
			/** Text shaping result
			 *
			 * This class can render a bloc of text. It can be generated to fit in a
			 * particular box.
			 */
			class Renderable {
				private:
					SDL::Point result_size;
					struct GlyphData {
						SDL::Point position;
						Glyph     *glyph;
						SDL_Color  color;
						inline GlyphData& operator=(const GlyphData& other) {
							position = other.position;
							glyph = other.glyph;
							color = other.color;
							return *this;
						}
						GlyphData(void) = default;
						inline GlyphData(SDL::Point position, Glyph &glyph, SDL_Color color)
						: position(position), glyph(&glyph), color(color) {}
					};
					std::vector<GlyphData> glyphs;
					void append_text(const std::string_view& text, SDL::Point &cursor, int wrap_width, Uint32 font_size, SDL_Color color);
					void append_text(const std::u32string_view& text, SDL::Point &cursor, int wrap_width, Uint32 font_size, SDL_Color color);
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
					Renderable(const Elements& elements, int wrap_width);
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
