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
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

using namespace std::literals::string_view_literals;
using namespace std::literals::string_literals;

namespace Arcollect {
	namespace gui {
		/** Arcollect text rendering infrastructure
		 *
		 * Arcollect render text using an internal engine built on top of FreeType2
		 * and HarfBuzz.
		 *
		 * When you render text, you pack multiple strings and modifers into a
		 * Arcollect::gui::font::Elements object using the `std::ostream <<` style.
		 * The Arcollect::gui::font::Elements describe logically what you display.
		 *
		 * Then, you generate a Arcollect::gui::font::Renderable that contain glyphs
		 * placement, colors and attributes using the Arcollect::gui::font::Elements
		 * (just pass it to it's constructor). It describe what is displayed and
		 * is oftenly cached.
		 * 
		 * \code
		 * Arcollect::gui::font::Elements elements;
		 * elements << U"Some text\n"sv << FontSize(42) << SDL::Color(255,0,0,255)
		 * << U"Big RED text!"sv;
		 * // Here we wrap text at 1373px width.
		 * Arcollect::gui::font::Renderable renderable(elements,1373);
		 * \endcode
		 *
		 * UTF-32 is used internally, you should use that with `U""sv` strings
		 * that yield a `std::u32string_view`. #Arcollect::gui::font::Elements have
		 * also a move-constructor from `std::u32string` that you get with a `U""sv`
		 * (this header include the required `using namespace` statements).
		 * Don't use UTF-32 outside the GUI and don't scream on UTF-8, it's fully
		 * supported and automatically converted.
		 */
		namespace font {
			class Renderable;
			#define Arcollect_gui_font_element_wrapper_boilerplate(TypeName) \
				constexpr operator decltype(value)&(void) {return value;} \
				constexpr operator const decltype(value)&(void) const {return value;} \
				constexpr TypeName& operator=(decltype(value) new_value) { \
					value = new_value; \
					return *this; \
				} \
				constexpr TypeName& operator=(const TypeName& new_value) { \
					value = new_value.value; \
					return *this; \
				} \
				constexpr TypeName(decltype(value) value) : value(value) {} \
				constexpr TypeName(const TypeName&) = default;
			struct FontSize {
				float value;
				Arcollect_gui_font_element_wrapper_boilerplate(FontSize)
				constexpr FontSize(void) : value(1) {}
			};
			constexpr FontSize ExactFontSize(float value) {
				return FontSize(-value);
			}
			/** Justification flag (bool)
			 *
			 * Append this to an Arcollect::gui::font::Elements to turn on/off
			 * text justification.
			 *
			 * This flag does not affect text alignment behavior.
			 *
			 * \warning The justification require the use of wrap_width when creating
			 *          the #Arcollect::gui::font::Renderable.
			 */
			struct Justify {
				bool value;
				Arcollect_gui_font_element_wrapper_boilerplate(Justify)
			};
			/** Alignment option
			 *
			 * \warning Alignment require the use of wrap_width when creating the
			 *          the #Arcollect::gui::font::Renderable.
			 *
			 * \todo Altough the API is text-direction aware. The implementation isn't
			 *       yet. #START is an alias for #LEFT and #END an alias for #RIGHT.
			 *
			 * \see Justification is handled separately, see #Justify.
			 */
			enum class Align {
				/** Align to the start of the line
				 *
				 * This is the default alignment option and take account of text
				 * direction (when supported).
				 *
				 * This is equivalent to #LEFT in LTR and #RIGHT in RTL.
				 */
				START,
				/** Align to the end of the line
				 *
				 * This is equivalent to #RIGHT in LTR and #LEFT in RTL.
				 */
				END,
				/** Align to the left
				 *
				 * In most case you will prefer #START that is text-direction aware.
				 */
				LEFT,
				/** Align to the centr
				 */
				CENTER,
				/** Align to the right
				 *
				 * In most case you will prefer #END that is text-direction aware.
				 */
				RIGHT,
			};
			/** Arcollect FreeType2 render flags
			 */
			static constexpr auto ft_flags = 0;
			
			FT_Face query_face(Uint32 font_size);
			/** Text rendering instructions buffer
			 *
			 * Used to ease text building with Elements::operator<<().
			 * Also contain initial state.
			 */
			class Elements {
				private:
					friend Renderable;
					/** Attributes description
					 *
					 * This structure contain text attributes description like the color.
					 * \note Some operations like font-size change require another shaping
					 *       call. #Attributes don't store these attributes which are
					 *       stored along the text run itself.
					 */
					struct Attributes {
						/** This attribute end/next attribute start index
						 *
						 * This field is used to know when we shoud change to the next
						 * attribute when rendering text.
						 *
						 * This value is the index when we should use the next attribute.
						 * A more natural logic would be to store when we start using this
						 * attribute but this require more code. Here we start when the next
						 * attribute start.
						 */
						decltype(hb_glyph_info_t::cluster) end;
						/** Alignment
						 */
						Align alignment;
						/** Justification flag
						 */
						Justify justify;
						/** Current glyph color
						 */
						SDL::Color color;
					};
					/** List of #Arcollect::gui::font::Elements::Attributes
					 *
					 * Contain the list of attributes in the order they appear
					 */
					std::vector<Attributes> attributes;
					/** List of text runs
					 *
					 * A text run is the shapable unit. HarfBuzz hb_shape() have a high
					 * cost for zero-length text. So we pack everything in a string to
					 * avoid making to avoid costful hb_shape().
					 */
					std::vector<std::pair<FontSize,std::u32string>> text_runs;
					/** If push_attribute() should push a new attribute
					 *
					 * This flag is raised in operator<<(const std::u32string_view&) and
					 * reset in push_attribute(). It avoid creating multiple
					 * #Arcollect::gui::font::Elements::Attributes when setting multiple
					 * attributes in a row.
					 */
					bool must_push_new_attribute = false;
					/** Get the #Attributes to change
					 * \return The #Attributes to change
					 *
					 * This function is called by operator<<() changing text attributes.
					 *
					 * It duplicate the back of #attributes, set
					 * #Arcollect::gui::font::Elements::Attributes::end and return a
					 * reference to it.
					 * Or it don't duplicate the back at all if useless.
					 */
					Attributes &push_attribute(void) {
						if (must_push_new_attribute) {
							attributes.emplace_back(attributes.back());
							must_push_new_attribute = false;
						}
						return attributes.back();
					}
					/** Start a new text run
					 * \return The new #text_runs element to set
					 *
					 * This function is called by operator<<() changing attributes that
					 * need a new text run.
					 *
					 * It's simmilar to push_attribute() and just return the #text_runs
					 * back if it has no text.
					 */
					decltype(text_runs)::value_type &push_text_run(void) {
						if (!text_runs.back().second.empty())
							text_runs.emplace_back(text_runs.back().first,U""s);
						return text_runs.back();
					}
					
				public:
					/** Change justification
					 *
					 * The current line justification is changed.
					 */
					Elements& operator<<(Justify justify) {
						push_attribute().justify = justify;
						return *this;
					}
					/** Change alignment
					 *
					 * The current line alignment is changed.
					 */
					Elements& operator<<(Align alignment) {
						push_attribute().alignment = alignment;
						return *this;
					}
					/** Change color
					 *
					 * The color change is immediate.
					 */
					Elements& operator<<(SDL::Color color) {
						push_attribute().color = color;
						return *this;
					}
					
					/** Change font size
					 *
					 * \warning Changing font size create another text run that 4ms on my
					 *          system. Use with caution!
					 */
					Elements& operator<<(FontSize font_size) {
						push_text_run().first = font_size;
						return *this;
					}
					/** Append UTF-32 text
					 *
					 * The string will be copied.
					 * \see The Elements(std::u32string&&,FontSize) move-constructor.
					 */
					Elements& operator<<(const std::u32string_view &string) {
						text_runs.back().second += string;
						attributes.back().end   += string.size();
						must_push_new_attribute = true;
						return *this;
					}
					/** Append UTF-8 text
					 *
					 * The string will be converted to UTF-32.
					 */
					Elements& operator<<(const std::string_view &string);
					
					/** Return if the #Elements is empty
					 *
					 * It is empty if it has no text inside.
					 * \note Packing attribute modifiers do not change empty() result.
					 */
					bool empty(void) {
						return text_runs[0].second.empty();
					}
					
					/** Parameter pack builder
					 * \param args Parameter pack
					 *
					 * Create a new #Elements by operator<<() all items of the parameter
					 * pack.
					 */
					Elements(void) :
						attributes{{0,
						// Initial attributes
						Align::LEFT,        // Alignment
						false,              // Justification
						{255,255,255,255}}, // Color
					},text_runs{{1,std::move(U""s)}},
						must_push_new_attribute(true) {
					}
					
					template <typename ... Args>
					static Elements build(Args... args) {
						return (Elements() << ... << args);
					}
			};
			
			/** Cached glyph
			 *
			 * This structure hold the renderered version of a character Glyph.
			 */
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
				static std::unordered_map<key,std::unique_ptr<Glyph>,key::hash> glyph_cache;
				/** Query a glyph
				 * \param key The glyph key
				 *
				 * Query and create on cache miss a glyph.
				 */
				static inline Glyph& query(const key& key) {
					auto iter = glyph_cache.find(key);
					if (iter == glyph_cache.end())
						iter = glyph_cache.emplace(key,std::make_unique<Glyph>(key)).first;
					return *(iter->second);
				}
				/** Query a glyph
				 *
				 * Query and create on cache miss a glyph.
				 */
				static inline Glyph& query(hb_codepoint_t glyphid, int font_size) {
					return query(key{glyphid,font_size});
				}
			};
			
			/** Text rendering configuration
			 */
			struct RenderConfig {
				/** Default font height in pixels
				 */
				int base_font_height;
				/** Justification forcing
				 */
				bool always_justify;
				RenderConfig(void);
			};
			
			/** Text rendering result
			 *
			 * This is what you render on the screen.
			 * It can be generated to fit in a particular box.
			 *
			 * \note Generating a #Renderable is costful (>4ms per
			 *       Arcollect::gui::font::Elements::text_run).
			 */
			class Renderable {
				private:
					SDL::Point result_size;
					struct GlyphData {
						SDL::Point position;
						Glyph     *glyph;
						SDL::Color  color;
						inline GlyphData& operator=(const GlyphData& other) {
							position = other.position;
							glyph = other.glyph;
							color = other.color;
							return *this;
						}
						GlyphData(void) = default;
						inline GlyphData(SDL::Point position, Glyph &glyph, SDL::Color color)
						: position(position), glyph(&glyph), color(color) {}
					};
					/** Glyph storage result
					 */
					std::vector<GlyphData> glyphs;
					/** Line data
					 */
					struct LineData {
						/** First point
						 */
						SDL::Point p0;
						/** Second point
						 */
						SDL::Point p1;
						/** Line color
						 */
						SDL::Color color;
					};
					/** Lines storage result
					 */
					std::vector<LineData> lines;
					/** Append a rectangle in #lines
					 * \param rect  The rectangle
					 * \param color Rectangle color
					 */
					void add_line(SDL::Point p0, SDL::Point p1, SDL::Color color) {
						lines.emplace_back(LineData{p0,p1,color});
					}
					/** Append a rectangle in #lines
					 * \param rect  The rectangle
					 * \param color Rectangle color
					 */
					void add_rect(const SDL::Rect &rect, SDL::Color color);
					
					struct RenderingState;
					/** Realign text
					 * \param align           The alignment to use
					 * \param i_start         The index of the first glyph to align
					 * \param i_end           The index of the last glyph to align (excluded)
					 * \param remaining_space The free space left on the line
					 */
					void align_glyphs(Align align, unsigned int i_start, unsigned int i_end, int remaining_space);
					/** Append a text run
					 */
					void append_text_run(const decltype(Elements::text_runs)::value_type& text_run, RenderingState &state);
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
					/** Rendert text
					 * \param elements Elements to add
					 * \param wrap_width The maximum width
					 * \param config The configuration to use
					 *
					 * \warning The wrap_width not an absolute limit!
					 */
					Renderable(const Elements& elements, int wrap_width = std::numeric_limits<int>::max(), const RenderConfig& config = RenderConfig());
					/** Render UTF-8 text
					 * \param text UTF-8 text
					 * \param font_size Text height
					 * \param wrap_width The maximum width
					 *
					 * \note The text will be convertd to UTF-32.
					 */
					Renderable(const std::string_view& text, FontSize font_size, int wrap_width = std::numeric_limits<int>::max())
						: Renderable(Elements() << font_size << text,wrap_width) {}
					Renderable(const Renderable&) = default;
					void render_tl(int x, int y);
					inline void render_tl(SDL::Point topleft_corner) {
						return render_tl(topleft_corner.x,topleft_corner.y);
					}
			};
		}
	}
}
