#include <arcollect-roboto.hpp>
#include "font.hpp"
#include <functional>
#include <hb-ft.h>
#include <locale>
#include <unordered_map>
#include FT_SIZES_H

extern SDL::Renderer *renderer;

static FT_Library ft_library;

const Arcollect::gui::font::FontSize Arcollect::gui::font::FontSize::normal(12);

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

void Arcollect::gui::font::Glyph::render(int origin_x, int origin_y, SDL_Color color) const
{
	SDL::Rect rect = coordinates;
	rect.x += origin_x;
	rect.y += origin_y;
	SDL_SetTextureColorMod((SDL_Texture*)text,color.r,color.g,color.b);
	SDL_SetTextureAlphaMod((SDL_Texture*)text,color.a);
	renderer->Copy(text,NULL,&rect);
}
std::unordered_map<Arcollect::gui::font::Glyph::key,std::unique_ptr<Arcollect::gui::font::Glyph>,Arcollect::gui::font::Glyph::key::hash> Arcollect::gui::font::Glyph::glyph_cache;

FT_Face Arcollect::gui::font::query_face(Uint32 font_size)
{
	static FT_Face face = NULL;
	if (!face) {
		FT_Init_FreeType(&ft_library);
		FT_New_Memory_Face(ft_library,(const FT_Byte*)Arcollect::Roboto::Light.data(),Arcollect::Roboto::Light.size(),0,&face);
	}
	Uint32 key{font_size};
	static std::unordered_map<decltype(key),FT_Size> cache;
	auto iter = cache.find(key);
	if (iter == cache.end()) {
		FT_Size new_size;
		FT_New_Size(face,&new_size);
		FT_Activate_Size(new_size);
		FT_Set_Pixel_Sizes(face,font_size,font_size);
		cache.emplace(key,new_size);
	} else FT_Activate_Size(iter->second);
	return face;
}

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

/** Current text attributes
 *
 * This structure is passed to append_text() and other functions and
 * is stored in a local variable of Renderable() constructor.
 */
struct Arcollect::gui::font::Renderable::RenderingState {
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
};
void Arcollect::gui::font::Renderable::append_text_run(const decltype(Elements::text_runs)::value_type& text_run, RenderingState &state)
{
	// Extract parameters
	const auto            font_size = text_run.first;
	const std::u32string_view& text = text_run.second;
	SDL::Point              &cursor = state.cursor;
	// Create the buffer
	hb_buffer_t *buf = hb_buffer_create();
	hb_buffer_pre_allocate(buf,text.size());
	hb_buffer_add_utf32(buf,reinterpret_cast<const uint32_t*>(text.data()),text.size(),0,-1);
	// FIXME Auto-detect better values
	hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
	hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language(buf, hb_language_from_string("en", -1));
	// Invoke Harfbuzz
	FT_Face face = query_face(font_size);
	const auto line_spacing = face->height >> 6;
	hb_font_t *font = hb_ft_font_create_referenced(face);
	hb_ft_font_set_load_flags(font,ft_flags);
	hb_shape(font,buf,NULL,0);
	hb_font_destroy(font);
	// Prepare glyphs process
	auto glyph_base = glyphs.size();
	unsigned int glyph_count;
	hb_glyph_info_t *glyph_infos    = hb_buffer_get_glyph_infos(buf, &glyph_count);
	hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
	glyphs.reserve(glyph_base+glyph_count);
	unsigned int glyphi_line_start = 0;
	auto clusteri_line_end = text.find(U'\n'); // To break at \n
	// Process glyphs
	for (unsigned int i = 0; i < glyph_count; i++) {
		// Extract rendering parameters
		if (state.attrib_iter->end <= i)
			++state.attrib_iter;
		const bool     &justify = state.attrib_iter->justify;
		const SDL_Color  &color = state.attrib_iter->color;
		
		hb_glyph_info_t &glyph_info = glyph_infos[i];
		glyph_pos[i].x_advance >>= 6;
		glyph_pos[i].y_advance >>= 6;
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
			// Justify text
			if (justify) {
				// TODO 
			}
			// Move glyphs on the newline
			cursor.x = 0;
			for (i_newline++; i_newline < i; i_newline++) {
				glyphs[glyph_base+i_newline].position.x  = cursor.x;
				glyphs[glyph_base+i_newline].position.y += line_spacing;
				cursor.x += glyph_pos[i_newline].x_advance;
			}
			// Move the cursor
			cursor.y += line_spacing;
			
			glyphi_line_start = i;
		} else if (glyph_info.cluster >= clusteri_line_end) {
			// We are on a line break '\n'
			cursor.x = -glyph_pos[i].x_advance;
			cursor.y += line_spacing;
			clusteri_line_end = text.find(U'\n',clusteri_line_end+1); // Find the next line break
			glyphi_line_start = i;
		}
		// Don't render blanks codepoints
		if (!((text[glyph_info.cluster] == U' ')
		 ||(text[glyph_info.cluster] == U'\t')
		 ||(text[glyph_info.cluster] == U'\r')
		 ||(text[glyph_info.cluster] == U'\n')
		 ||(text[glyph_info.cluster] == 0x00A0)) // nbsp
		 ) {
			// Emplace the new char
			auto &glyph = Glyph::query(glyph_info.codepoint,font_size);
			glyphs.emplace_back(cursor,glyph,color);
			// Update bound
			result_size.x = std::max(result_size.x,static_cast<int>(cursor.x+glyph.coordinates.x+glyph.coordinates.w));
			result_size.y = std::max(result_size.y,static_cast<int>(cursor.y+glyph.coordinates.y+glyph.coordinates.h));
		} else glyph_base--;
		// Update cursor
		cursor.x += glyph_pos[i].x_advance;
		cursor.y += glyph_pos[i].y_advance;
	}
	// Cleanups
	hb_buffer_destroy(buf);
}
Arcollect::gui::font::Renderable::Renderable(const Elements& elements, int wrap_width) :
	result_size{0,0}
{
	RenderingState state {
		{0,0},// cursor
		wrap_width,
		elements.attributes.begin(),
	};
	for (const auto& text_run: elements.text_runs)
		append_text_run(text_run,state);
}
void Arcollect::gui::font::Renderable::render_tl(int x, int y)
{
	for (const GlyphData& glyph: glyphs)
		glyph.glyph->render(glyph.position.x + x, glyph.position.y + y, glyph.color);
}
