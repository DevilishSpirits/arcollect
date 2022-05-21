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
#include <arcollect-debug.hpp>
#include "font-internal.hpp" // #include "font.hpp"
#include <locale>

extern SDL::Renderer *renderer;
FT_Library Arcollect::gui::font::ft_library;

void Arcollect::gui::font::init(void)
{
	FT_Init_FreeType(&Arcollect::gui::font::ft_library);
	Arcollect::gui::font::os_init();
}

Arcollect::gui::font::Glyph::Glyph(hb_codepoint_t glyphid, FT_Face face)
{
	// Render glyph
	FT_Load_Glyph(face,glyphid,ft_flags|FT_LOAD_RENDER);
	// Copy bitmap
	FT_Bitmap &bitmap = face->glyph->bitmap;
	SDL::Surface* surf = (SDL::Surface*)SDL_CreateRGBSurfaceWithFormat(0,bitmap.width,bitmap.rows,32,SDL_PIXELFORMAT_RGBA32);
	for (unsigned int y = 0; y < bitmap.rows; y++) {
		char *pixels_line = &reinterpret_cast<char*>(surf->pixels)[surf->pitch*y];
		for (unsigned int x = 0; x < bitmap.width; x++)
			reinterpret_cast<Uint32*>(pixels_line)[x] = SDL_MapRGBA(surf->format,255,255,255,bitmap.buffer[y*bitmap.pitch+x]);
	}
	text = SDL::Texture::CreateFromSurface(renderer,surf);
	delete surf;
	// Set coordinates
	coordinates.x = face->glyph->bitmap_left;
	coordinates.y = (face->size->metrics.height >> 6) - face->glyph->bitmap_top;
	coordinates.w = bitmap.width;
	coordinates.h = bitmap.rows;
}
Arcollect::gui::font::Glyph::~Glyph(void) {
	delete text;
}

void Arcollect::gui::font::Glyph::render(int origin_x, int origin_y, SDL::Color color) const
{
	SDL::Rect rect = coordinates;
	rect.x += origin_x;
	rect.y += origin_y;
	SDL_SetTextureColorMod((SDL_Texture*)text,color.r,color.g,color.b);
	SDL_SetTextureAlphaMod((SDL_Texture*)text,color.a);
	renderer->Copy(text,NULL,&rect);
}
std::unordered_map<std::size_t,Arcollect::gui::font::Glyph> Arcollect::gui::font::Glyph::glyph_cache;

Arcollect::gui::font::Elements& Arcollect::gui::font::Elements::operator<<(const std::string_view &string)
{
	typedef std::codecvt<char32_t,char/*char8_t*/,std::mbstate_t> codecvt_t;
	// Convert the string in UTF-32
	std::u32string codepoints;
	codepoints.resize(string.size());
	static const codecvt_t &codecvt = std::use_facet<codecvt_t>(std::locale());
	const char/*char8_t*/* trash8;
	char32_t* utf32_end;
	std::mbstate_t codecvt_state;
	codecvt.in(codecvt_state,&*(string.begin()),&*(string.end()),trash8,&*(codepoints.begin()),&*(codepoints.end()),utf32_end);
	codepoints.resize(std::distance(codepoints.data(),utf32_end));
	return operator<<(codepoints);
}


Arcollect::gui::font::RenderConfig::RenderConfig() :
	base_font_height(14),
	always_justify(false)
{
}


void Arcollect::gui::font::Renderable::add_rect(const SDL::Rect &rect, SDL::Color color)
{
	const auto left  = rect.x;
	const auto top   = rect.y;
	const auto right = rect.x + rect.w;
	const auto bot   = rect.y + rect.h;
	add_line({left ,top},{right,top},color);
	add_line({right,top},{right,bot},color);
	add_line({right,bot},{left ,bot},color);
	add_line({left ,bot},{left ,top},color);
}

void Arcollect::gui::font::Renderable::align_glyphs(RenderingState &state, int remaining_space)
{
	Align align = state.attrib_iter->alignment;
	// Update the line 
	const auto start = state.line_first_glyph_index;
	state.line_first_glyph_index = glyphs.size();
	// Handle align
	switch (align) {
		/*case Align::START: {
			TODO 
		} break;*/
		/*case Align::END: {
			TODO 
		} break;*/
		case Align::START: // TODO RTL support
		case Align::LEFT: {
			// Do nothing, we align to left by default
		} return;
		case Align::CENTER: {
			remaining_space >>= 1;
		} break;
		case Align::END: // TODO RTL support
		case Align::RIGHT: {
			// Just pack right
		} break;
	}
	// Realign
	for (auto i = start; i < glyphs.size(); ++i)
		glyphs[i].position.x += remaining_space;
	// Debug
	if (Arcollect::debug.fonts) {
		const SDL::Point& lgp = glyphs[start].position; // Left glyph position
		const SDL::Point& ilgp{lgp.x-remaining_space,lgp.y}; // Initial left glyph position
		const SDL::Point& rgp = glyphs.back().position; // Right glyph position
		constexpr SDL::Color color{{255,0,0,128}};
		constexpr int length = 8; // Length of small things
		// Draw a 'A'
		constexpr int A_height = length/2;
		constexpr int A_width  = A_height/2;
		add_line({ilgp.x-A_width,ilgp.y+A_height},{ilgp.x+  A_width,ilgp.y-A_height},color);
		add_line({ilgp.x+A_width,ilgp.y-A_height},{ilgp.x+3*A_width,ilgp.y+A_height},color);
		// Draw by how much we moved the text
		add_line(ilgp,lgp,color);
		// Draw the left limit
		add_line(lgp,{lgp.x,lgp.y+length},color);
		if (align == Align::CENTER) {
			// Draw the right limit
			add_line(rgp,{rgp.x+remaining_space,rgp.y},color);
			add_line(rgp,{rgp.x,rgp.y+length},color);
			add_line({rgp.x+remaining_space,rgp.y},{rgp.x+remaining_space,rgp.y+length},color);
		}
	}
}
void Arcollect::gui::font::Renderable::append_text_run(const unsigned int cp_offset, int cp_count, RenderingState &state, Arcollect::gui::font::shape_data *shape_data)
{
	// Extract parameters
	decltype(Elements::attributes)::const_iterator &attrib_iter = state.attrib_iter;
	state.font_height = (attrib_iter->font_size.value < 0 ? -attrib_iter->font_size.value : attrib_iter->font_size.value * state.config.base_font_height); // TODO Make the size customizable
	const std::u32string_view &text = state.text;
	SDL::Point              &cursor = state.cursor;
	// Create the buffer
	hb_buffer_t *buf = hb_buffer_create();
	hb_buffer_pre_allocate(buf,text.size());
	hb_buffer_add_utf32(buf,reinterpret_cast<const uint32_t*>(state.text.data()),state.text.size(),cp_offset,cp_count);
	// Invoke Harfbuzz
	FT_Face face = Arcollect::gui::font::shape_hb_buffer(state,buf,shape_data);
	const auto font_line_skip = face->size->metrics.height >> 6;
	// Prepare glyphs process
	auto glyph_base = glyphs.size();
	unsigned int glyph_count;
	unsigned int skiped_glyph_count = 0;
	hb_glyph_info_t *glyph_infos    = hb_buffer_get_glyph_infos(buf, &glyph_count);
	hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
	glyphs.reserve(glyph_base+glyph_count);
	unsigned int glyphi_line_start = 0;
	// Process leading '\n' (avoid a SEGFAULT in the code)
	for (;(glyphi_line_start < glyph_count)&&(text[glyphi_line_start+cp_offset] == '\n'); ++glyphi_line_start)
		state.skip_line(font_line_skip);
	
	if (state.current_line_skip < font_line_skip)
		state.current_line_skip = font_line_skip;
	
	auto clusteri_line_end = text.find(U'\n',cp_offset+(glyphi_line_start ? glyphi_line_start+1 : 0)); // To break at \n
	// Process glyphs
	for (unsigned int i = glyphi_line_start; i < glyph_count; i++) {
		hb_glyph_info_t &glyph_info = glyph_infos[i];
		glyph_pos[i].x_advance >>= 6;
		glyph_pos[i].y_advance >>= 6;
		const char32_t glyph_char  = text[glyph_info.cluster];
		// Extract rendering parameters
		if (state.attrib_iter->end <= glyph_info.cluster)
			++state.attrib_iter;
		const bool      justify = state.config.always_justify || state.attrib_iter->justify;
		const SDL::Color  &color = state.attrib_iter->color;
		// Wrap text (but not if we already started a new_line, the cluster won't fit anyway)
		if ((((cursor.x + glyph_pos[i].x_advance) > state.wrap_width) && (glyph_info.cluster != glyph_infos[glyphi_line_start].cluster))) {
			// Search backward to a safe cluster and char to wrap
			unsigned int i_newline;
			for (i_newline = i; i_newline; i_newline--) {
				auto char_i = glyph_infos[i_newline].cluster;
				if ((text[char_i] == U' ')||(text[char_i] == U'\t')) {
					break;
				}
			}
			if (!i_newline)
				for (i_newline = i; i_newline; i_newline--) {
					auto char_i = glyph_infos[i_newline].cluster;
					if ((text[char_i] == U':')||(text[char_i] == U'/')||(text[char_i] == U'\\')||(text[char_i] == U'.')||(text[char_i] == U',')||(text[char_i] == U';')) {
						break;
					}
				}
			// TODO Handle when we can't break on a space (this code avoid a SEGFAULTs)
			if (!i_newline)
				break;
			// Justify/align text
			int right_free_space = state.wrap_width - glyph_pos[i_newline-1].x_advance;
			if (i_newline == i)
				// We are breaking on a space, i_newline point to an non existing char
				right_free_space -= glyphs[glyph_base+i_newline-1].position.x;
			else right_free_space -= glyphs[glyph_base+i_newline].position.x;
			if (justify) {
				// Count the number of spaces in the line
				unsigned int space_count = 0;
				for (auto j = i_newline-1; j > glyphi_line_start; j--) {
					auto char_j = glyph_infos[j].cluster;
					if ((text[char_j] == U' ')||(text[char_j] == U'\t')||(glyph_char == 0x00A0/*nbsp*/))
						space_count++;
				}
				// Avoid division by zero
				if (!space_count)
					space_count = 1;
				// Compute pixel delta
				int glyph_id_delta = glyph_base+skiped_glyph_count;
				unsigned int space_current = 0;
				int pixel_delta = 0;
				// Move glyphs
				for (auto j = glyphi_line_start; j < i_newline; j++) {
					auto char_j = glyph_infos[j].cluster;
					if ((text[char_j] == U' ')||(text[char_j] == U'\t')||(glyph_char == 0x00A0/*nbsp*/)) {
						space_current++;
						glyph_id_delta--;
						pixel_delta = right_free_space*space_current/space_count;
						if (Arcollect::debug.fonts) {
							const auto &last_pos = glyphs[glyph_id_delta+j].position;
							add_line(last_pos,{last_pos.x,static_cast<int>(last_pos.y+state.current_line_skip)},{255,0,0,255});
						}
					} else glyphs[glyph_id_delta+j].position.x += pixel_delta;
				}
			} else align_glyphs(state,right_free_space);
			// Adjust line_first_glyph_index
			state.line_first_glyph_index -= i-i_newline-1;
			// Move glyphs on the newline
			cursor.x = 0;
			for (i_newline++; i_newline < i; i_newline++) {
				glyphs[glyph_base+i_newline].position.x  = cursor.x;
				glyphs[glyph_base+i_newline].position.y += state.current_line_skip;
				cursor.x += glyph_pos[i_newline].x_advance;
			}
			// Update line start index and cursor
			skiped_glyph_count = 0;
			glyphi_line_start  = i_newline+1;
			state.skip_line(font_line_skip);
			// Skip the current char if it's a space
			if ((glyph_char == U' ')||(glyph_char == U'\t')) {
				glyph_base--;
				continue;
			}
		} else if (glyph_info.cluster >= clusteri_line_end) {
			// We are on a line break '\n'
			int right_free_space = state.wrap_width - cursor.x + glyph_pos[i-1].x_advance;
			align_glyphs(state,right_free_space);
			cursor.x = -glyph_pos[i].x_advance;
			state.skip_line(font_line_skip);
			clusteri_line_end = text.find(U'\n',clusteri_line_end+1+cp_offset); // Find the next line break
			glyphi_line_start = i+1;
			skiped_glyph_count = -1; // Will be incremented back a few lines later
		}
		// Don't render blanks codepoints
		if (!((glyph_char <= 0x20) // Space and ASCII control codes
		 ||(glyph_char == 0x00A0)) // nbsp
		 ) {
			// Emplace the new char
			auto &glyph = Glyph::query(glyph_info.codepoint,face);
			glyphs.emplace_back(cursor,glyph,color);
			// Update bound
			result_size.x = std::max(result_size.x,static_cast<int>(cursor.x+glyph.coordinates.x+glyph.coordinates.w));
			result_size.y = std::max(result_size.y,static_cast<int>(cursor.y+glyph.coordinates.y+glyph.coordinates.h));
		} else {
			glyph_base--;
			skiped_glyph_count++;
		}
		// Update cursor
		cursor.x += glyph_pos[i].x_advance;
		cursor.y += glyph_pos[i].y_advance;
	}
	// Cleanups and late updates
	hb_buffer_destroy(buf);
	state.text_run_cluster_offset += glyph_count;
}
Arcollect::gui::font::Renderable::Renderable(const Elements& elements, int wrap_width, const RenderConfig& config) :
	result_size{0,0}
{
	// Init rendering state
	RenderingState state {
		config,
		elements.text,
		{0,0},// cursor
		wrap_width,
		elements.attributes.begin(),
		0, // line_first_glyph_index
		0,// text_run_cluster_offset
	};
	// Render text
	unsigned int cp_offset = 0;
	while (cp_offset < elements.text.size()) {
		// Perform text run
		Arcollect::gui::font::shape_data *shape_data;
		int cp_count = Arcollect::gui::font::text_run_length(state,cp_offset,shape_data);
		// Step attrib_iter if we're on the end
		// Note: It's after the text_run_length function to avoid skipping attrib changes
		if (state.attrib_iter->end <= cp_offset)
			++state.attrib_iter;
		// Perform text run
		append_text_run(cp_offset,cp_count,state,shape_data);
		cp_offset += cp_count;
	}
	// Align the rest
	state.attrib_iter = --elements.attributes.end(); // Get back on a correct attrib_iter
	align_glyphs(state,state.wrap_width - state.cursor.x);
	
	// Outline the text in debug mode
	if (Arcollect::debug.fonts)
		add_rect({0,0,result_size.x,result_size.y},{255,255,255,128});
	// Shrink vectors
	glyphs.shrink_to_fit();
	lines.shrink_to_fit();
}
void Arcollect::gui::font::Renderable::render_tl(int x, int y)
{
	for (const GlyphData& glyph: glyphs)
		glyph.glyph->render(glyph.position.x + x, glyph.position.y + y, glyph.color);
	for (const LineData& line: lines) {
		renderer->SetDrawColor(line.color);
		renderer->DrawLine(line.p0.x + x, line.p0.y + y,line.p1.x + x, line.p1.y + y);
	}
}
