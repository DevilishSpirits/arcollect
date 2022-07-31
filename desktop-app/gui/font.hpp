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
#include <limits>
#include <string>
#include <string_view>
#include <vector>

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
			/** Init the font rendering engine
			 */
			void init(void);
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
			/** Bolden flag (int)
			 *
			 * Append this to an Arcollect::gui::font::Elements to turn on/off
			 * text justification.
			 *
			 * This flag does not affect text alignment behavior.
			 *
			 * \warning The justification require the use of wrap_width when creating
			 *          the #Arcollect::gui::font::Renderable.
			 */
			struct Weight {
				int value;
				Arcollect_gui_font_element_wrapper_boilerplate(Weight)
				constexpr Weight(void) : value(80) {}
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
			/** Attributes description
			 *
			 * This structure contain text attributes description like the color.
			 *
			 * It is never manipulated directly in user code.
			 */
			struct Attributes {
				/** This attribute end/next attribute start index
				 *
				 * This field is used to know when we should change to the next
				 * attribute when rendering text.
				 *
				 * This value is the index when we should use the next attribute.
				 * A more natural logic would be to store when we start using this
				 * attribute but this require more code. Here we start when the next
				 * attribute start.
				 */
				uint32_t end;
				/** Alignment
				 */
				Align alignment;
				/** Justification flag
				 */
				Justify justify;
				/** Text weight
				 */
				Weight weight;
				/** Current glyph color
				 */
				SDL::Color color;
				/** Font size
				 */
				FontSize font_size;
			};
			/** Text rendering instructions buffer
			 *
			 * Used to ease text building with Elements::operator<<().
			 * Also contain initial state.
			 */
			class Elements {
				private:
					friend Renderable;
					/** List of #Arcollect::gui::font::Elements::Attributes
					 *
					 * Contain the list of attributes in the order they appear
					 */
					std::vector<Attributes> attributes;
					/** Text storage
					 *
					 * This store the whole text in UTF-32
					 */
					std::u32string text;
					/** If push_attribute() should push a new attribute
					 *
					 * This flag is raised in operator<<(const std::u32string_view&) and
					 * reset in push_attribute(). It avoid creating multiple
					 * #Arcollect::gui::font::Elements::Attributes when setting multiple
					 * attributes in a row.
					 */
					bool must_push_new_attribute;
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
				public:
					/** Change justification
					 *
					 * The current line justification is changed.
					 */
					Elements& operator<<(Justify justify) {
						push_attribute().justify = justify;
						return *this;
					}
					/** Change justification
					 *
					 * The current line justification is changed.
					 */
					Elements& operator<<(Weight weight) {
						push_attribute().weight = weight;
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
					 */
					Elements& operator<<(FontSize font_size) {
						push_attribute().font_size = font_size;
						return *this;
					}
					/** Append UTF-32 text
					 *
					 * The string will be copied.
					 * \see The Elements(std::u32string&&,FontSize) move-constructor.
					 */
					Elements& operator<<(const std::u32string_view &string) {
						text += string;
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
					bool empty(void) const {
						return text.empty();
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
						Weight(),           // Weight
						{255,255,255,255},  // Color
						FontSize(1),        // FontSize
					}},
						must_push_new_attribute(false) {
					}
					
					template <typename ... Args>
					static Elements build(Args... args) {
						return (Elements() << ... << args);
					}
					/** Dump the content to stderr
					 *
					 * Debugging function to render Elements right down to the terminal.
					 * \note It requires a TrueColor capable terminal to works.
					 */
					void dump_to_stderr(void) const;
			};
			
			/** Cached glyph
			 *
			 * This structure hold the renderered version of a character Glyph.
			 */
			struct Glyph;
			
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
			
			/** Opaque struct for implementations usage
			 */
			struct shape_data;
			/** Text rendering result
			 *
			 * This is what you render on the screen.
			 * It can be generated to fit in a particular box.
			 */
			class Renderable {
				public:
					struct RenderingState;
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
					
					/** Realign text
					 * \param state           The rendering state
					 * \param remaining_space The free space left on the line
					 */
					void align_glyphs(RenderingState &state, int remaining_space);
					/** Append a text run
					 * \param cp_offset Offset to start shaping in the text
					 * \param cp_count Number of codepoints to shape
					 * \param state The rendering state
					 *
					 * A text run is a batch for text shaping.
					 */
					void append_text_run(unsigned int cp_offset, int cp_count, RenderingState &state, Arcollect::gui::font::shape_data *shape_data);
					
					/** Render text real function
					 * \param attrib_begin Start of attributes array
					 * \param attrib_count Length ofattributes array
					 * \param text to render
					 * \param wrap_width The maximum width
					 * \param config The configuration to use
					 *
					 * Real Elements class independant constructor.
					 * 
					 * \warning The wrap_width not an absolute limit!
					 */
					Renderable(const Attributes* attrib_begin, std::size_t attrib_count, std::u32string_view text, int wrap_width, const RenderConfig& config);
				public:
					inline const SDL::Point size() const {
						return result_size;
					}
					/** Renderable empty constructor
					 *
					 * \warning The object is invalid and trying to render is undefined.
					 *          You must set the object to something valid before render.
					 */
					Renderable(void) = default;
					/** Render text
					 * \param elements Elements to add
					 * \param wrap_width The maximum width
					 * \param config The configuration to use
					 *
					 * \warning The wrap_width not an absolute limit!
					 */
					Renderable(const Elements& elements, int wrap_width = std::numeric_limits<int>::max(), const RenderConfig& config = RenderConfig()) :
						Renderable(elements.attributes.data(),elements.attributes.size(),elements.text,wrap_width,config) {}
					/** Render undelimited text
					 * \param elements Elements to add
					 * \param config The configuration to use
					 *
					 */
					Renderable(const Elements& elements, const RenderConfig& config) : Renderable(elements,std::numeric_limits<int>::max(),config) {}
					Renderable(const Renderable&) = default;
					void render_tl(int x, int y) const;
					inline void render_tl(SDL::Point topleft_corner) const {
						return render_tl(topleft_corner.x,topleft_corner.y);
					}
					inline void render_cl(int x, int y, int h) const {
						return render_tl(x,y+(h-result_size.y)/2);
					}
			};
		}
	}
}
