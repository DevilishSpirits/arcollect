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
#include <hb-ft.h>
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
	/** Height of the font in pixels
	 */
	FT_UInt font_height;
};

namespace Arcollect {
	namespace gui {
		namespace font {
			extern FT_Library ft_library;
			/** System specific 
			 * 
			 */
			void os_init(void);
			/** Arcollect FreeType2 render flags
			 */
			static constexpr auto ft_flags = 0;
			
			/** FT_Face->generic commons
			 *
			 * Each FT_Face generic point to a FaceGeneric.
			 * Platform specific implementation may append data fields and have to
			 * set themselves the destructor of the FT_Generic.
			 */
			struct FaceGeneric {
				/** Hashing key of the face
				 * 
				 * This is a hashing key of the face configuration.
				 * For seasoned FreeType users, this is your FTC_FaceID.
				 */
				std::size_t hash_key;
			};
			
			struct Glyph {
				SDL::Texture* text;
				SDL::Rect coordinates;
				
				/** Create a glyph
				 *
				 * The glyph is rendered and cached into a SDL::Texture.
				 *
				 * \note Use query() instead that use the cache.
				 */
				Glyph(hb_codepoint_t glyphid, FT_Face face);
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
				/** Glyph render cache
				 */
				static std::unordered_map<std::size_t,Glyph> glyph_cache;
				/** Query a glyph
				 * \param glyphid to query
				 * \param data from the shaping
				 *
				 * Query and create on cache miss a glyph.
				 */
				static inline Glyph& query(hb_codepoint_t glyphid, FT_Face face) {
					const FaceGeneric& generic = *static_cast<const FaceGeneric*>(face->generic.data);
					return glyph_cache.try_emplace(std::hash<decltype(glyphid)>()(glyphid)^generic.hash_key,glyphid,face).first->second;
				}
			};
			/** Perform configuration and shaping of the buffer
			 * \param state          to use
			 * \param[in/out] buffer to shape
			 * \return Loaded FT_Face (do not add a extra reference)
			 *
			 * This part configure the buffer and invoke hb_shape() on the buffer.
			 * You MUST set the returned FT_Face generic to a hash value of the state.
			 * \note The implementation of this function is platform specific.
			 */
			FT_Face shape_hb_buffer(const Arcollect::gui::font::Renderable::RenderingState& state, hb_buffer_t *buffer);
		}
	}
}
