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
#include <functional>
#include <hb-ft.h>
#include <locale>
#include <unordered_map>

extern SDL::Renderer *renderer;

Arcollect::gui::font::Glyph::Glyph(hb_codepoint_t glyphid, int font_size)
{
	FT_Face face = query_face(font_size);
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
	coordinates.y = font_size - face->glyph->bitmap_top;
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
std::unordered_map<Arcollect::gui::font::Glyph::key,Arcollect::gui::font::Glyph,Arcollect::gui::font::Glyph::key::hash> Arcollect::gui::font::Glyph::glyph_cache;

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

void Arcollect::gui::font::Renderable::align_glyphs(Align align, unsigned int i_start, unsigned int i_end, int remaining_space)
{
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
	for (auto i = i_start; i < i_end; i++)
		glyphs[i].position.x += remaining_space;
	// Debug
	if (Arcollect::debug.fonts) {
		const SDL::Point& lgp = glyphs[i_start].position; // Left glyph position
		const SDL::Point& ilgp{lgp.x-remaining_space,lgp.y}; // Initial left glyph position
		const SDL::Point& rgp = glyphs[i_end-1].position; // Right glyph position
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
void Arcollect::gui::font::Renderable::append_text_run(const decltype(Elements::text_runs)::value_type& text_run, RenderingState &state)
{
	// Extract parameters
	const int             font_size = text_run.first < 0 ? -text_run.first : text_run.first * state.config.base_font_height; // TODO Make the size customizable
	const std::u32string_view& text = text_run.second;
	SDL::Point              &cursor = state.cursor;
	// Create the buffer
	hb_buffer_t *buf = hb_buffer_create();
	hb_buffer_pre_allocate(buf,text.size());
	hb_buffer_add_utf32(buf,reinterpret_cast<const uint32_t*>(text.data()),text.size(),0,text.size());
	// FIXME Auto-detect better values
	hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
	hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language(buf, hb_language_from_string("en", -1));
	// Invoke Harfbuzz
	FT_Face face = query_face(font_size);
	const auto line_spacing = face->size->metrics.height >> 6;
	hb_font_t *font = hb_ft_font_create_referenced(face);
	hb_ft_font_set_load_flags(font,ft_flags);
	hb_shape(font,buf,NULL,0);
	hb_font_destroy(font);
	// Prepare glyphs process
	auto glyph_base = glyphs.size();
	unsigned int glyph_count;
	unsigned int skiped_glyph_count = 0;
	hb_glyph_info_t *glyph_infos    = hb_buffer_get_glyph_infos(buf, &glyph_count);
	hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
	glyphs.reserve(glyph_base+glyph_count);
	unsigned int glyphi_line_start = 0;
	// Process leading '\n' (avoid a SEGFAULT in the code)
	for (;(glyphi_line_start < glyph_count)&&(text[glyphi_line_start] == '\n'); ++glyphi_line_start)
		cursor.y += line_spacing;
	
	auto clusteri_line_end = text.find(U'\n',glyphi_line_start ? glyphi_line_start+1 : 0); // To break at \n
	// Process glyphs
	for (unsigned int i = glyphi_line_start; i < glyph_count; i++) {
		hb_glyph_info_t &glyph_info = glyph_infos[i];
		glyph_pos[i].x_advance >>= 6;
		glyph_pos[i].y_advance >>= 6;
		const char32_t glyph_char  = text[glyph_info.cluster];
		// Extract rendering parameters
		if (state.attrib_iter->end <= glyph_info.cluster + state.text_run_cluster_offset)
			++state.attrib_iter;
		const Align  &alignment = state.attrib_iter->alignment;
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
							add_line(last_pos,{last_pos.x,last_pos.y+font_size},{255,0,0,255});
						}
					} else glyphs[glyph_id_delta+j].position.x += pixel_delta;
				}
			} else align_glyphs(alignment,glyph_base+glyphi_line_start+skiped_glyph_count,glyph_base+i_newline+1,right_free_space);
			// Update line start index and cursor
			skiped_glyph_count = 0;
			glyphi_line_start  = i_newline+1;
			cursor.y          += line_spacing;
			// Move glyphs on the newline
			cursor.x = 0;
			for (i_newline++; i_newline < i; i_newline++) {
				glyphs[glyph_base+i_newline].position.x  = cursor.x;
				glyphs[glyph_base+i_newline].position.y += line_spacing;
				cursor.x += glyph_pos[i_newline].x_advance;
			}
			// Skip the current char if it's a space
			if ((glyph_char == U' ')||(glyph_char == U'\t')) {
				glyph_base--;
				continue;
			}
		} else if (glyph_info.cluster >= clusteri_line_end) {
			// We are on a line break '\n'
			int right_free_space = state.wrap_width - cursor.x + glyph_pos[i-1].x_advance;
			align_glyphs(alignment,glyph_base+glyphi_line_start+skiped_glyph_count,glyph_base+i,right_free_space);
			cursor.x = -glyph_pos[i].x_advance;
			cursor.y += line_spacing;
			clusteri_line_end = text.find(U'\n',clusteri_line_end+1); // Find the next line break
			glyphi_line_start = i+1;
			skiped_glyph_count = -1; // Will be incremented back a few lines later
		}
		// Don't render blanks codepoints
		if (!((glyph_char <= 0x20) // Space and ASCII control codes
		 ||(glyph_char == 0x00A0)) // nbsp
		 ) {
			// Emplace the new char
			auto &glyph = Glyph::query(glyph_info.codepoint,font_size);
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
	// Align the rest
	align_glyphs(state.attrib_iter->alignment,glyph_base+glyphi_line_start+skiped_glyph_count,glyph_base+glyph_count,state.wrap_width - cursor.x);
	// Cleanups and late updates
	hb_buffer_destroy(buf);
	state.text_run_cluster_offset += glyph_count;
}
Arcollect::gui::font::Renderable::Renderable(const Elements& elements, int wrap_width, const RenderConfig& config) :
	result_size{0,0}
{
	RenderingState state {
		config,
		{0,0},// cursor
		wrap_width,
		elements.attributes.begin(),
		0,// text_run_cluster_offset
	};
	for (const auto& text_run: elements.text_runs)
		append_text_run(text_run,state);
	if (Arcollect::debug.fonts)
		add_rect({0,0,result_size.x,result_size.y},{255,255,255,128});
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
