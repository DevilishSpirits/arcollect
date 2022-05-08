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
/** \file font-internal.hpp
 *  \brief Internal header of the font rendering engine
 *
 * Don't use it outside the text rendering API itself.
 */
#pragma once
#include "font.hpp"
#include <hb.h>
#include <unordered_map>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

/** Current text attributes
 *
 * This structure is passed to append_text() and other functions and
 * is stored in a local variable of Renderable() constructor.
 */
struct Arcollect::gui::font::Renderable::RenderingState {
	/** Rendering configuration
	 */
	const RenderConfig& config;
	/** Current cursor position
	 *
	 * It's the "pen" in FreeType2 vocabulary.
	 */
	SDL::Point cursor;
	/** Text wrap_width
	 *
	 * It is set to a large value when no wrapping is wanted.
	 */
	const int wrap_width;
	/** Attribute iterator
	 *
	 * This is an iterator to the Arcollect::gui::font::Elements::attributes.
	 * It is incremented when needed by
	 * Arcollect::gui::font::Renderable::append_text_run().
	 */
	decltype(Elements::attributes)::const_iterator attrib_iter;
	/** Number of clusters wrote in previous text runs
	 *
	 * It is required to keep track of attrib_iter indexes
	 */
	unsigned int text_run_cluster_offset;
};

namespace Arcollect {
	namespace gui {
		namespace font {
			/** System specific 
			 * 
			 */
			void os_init(void);
			/** Arcollect FreeType2 render flags
			 */
			static constexpr auto ft_flags = 0;
			
			FT_Face query_face(Uint32 font_size);
			
			struct Glyph {
				SDL::Texture* text;
				SDL::Rect coordinates;
				
				/** Key for #glyph_cache
				 *
				 * This is an internal helper structure.
				 */
				struct key {
					hb_codepoint_t glyphid;
					int font_size;
					struct hash {
						std::size_t operator()(const key& key) const {
							return (std::hash<hb_codepoint_t>()(key.glyphid) << 1) ^ std::hash<int>()(key.font_size);
						}
					};
					constexpr bool operator==(const key& other) const {
						return (glyphid == other.glyphid)&&(font_size == other.font_size);
					}
				};
				
				/** Create a glyph
				 *
				 * The glyph is rendered and cached into a SDL::Texture.
				 *
				 * \note Use query() instead that use the cache.
				 */
				Glyph(hb_codepoint_t glyphid, int font_size);
				/** Create a glyph from key
				 *
				 * A simple shortcut
				 *
				 * \note Use query() instead that use the cache.
				 */
				Glyph(const key& key) : Glyph(key.glyphid,key.font_size) {}
				~Glyph(void);
				
				/** Render the glyph
				 * \param origin_x The cursor X position
				 * \param origin_y The cursor Y position
				 * \param color    The color to render
				 *
				 * render(int,int,SDL::Color) wrapper for SDL::Point
				 */
				void render(int origin_x, int origin_y, SDL::Color color) const;
				/** Render the glyph
				 * \param origin The cursor position
				 * \param color  The cursor position
				 * \param color    The color to render
				 *
				 * render(int,int,SDL::Color) wrapper for SDL::Point
				 */
				void render(SDL::Point origin, SDL::Color color) const {
					return render(origin.x,origin.y,color);
				}
				// Glyph cache
				/** Glyph render cache
				 */
				static std::unordered_map<key,Glyph,key::hash> glyph_cache;
				/** Query a glyph
				 * \param key The glyph key
				 *
				 * Query and create on cache miss a glyph.
				 */
				static inline Glyph& query(const key& key) {
					auto iter = glyph_cache.find(key);
					if (iter == glyph_cache.end())
						iter = glyph_cache.emplace(key,key).first;
					return iter->second;
				}
				/** Query a glyph
				 *
				 * Query and create on cache miss a glyph.
				 */
				static inline Glyph& query(hb_codepoint_t glyphid, int font_size) {
					return query(key{glyphid,font_size});
				}
			};
		}
	}
}
