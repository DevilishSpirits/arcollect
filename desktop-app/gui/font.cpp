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
	// Set delta
	delta.x = face->glyph->bitmap_left;
	delta.y = font_size - face->glyph->bitmap_top;
	// Copy bitmap
	FT_Bitmap_Init(&bitmap);
	FT_Bitmap_Copy(ft_library,&face->glyph->bitmap,&bitmap);
}
Arcollect::gui::font::Glyph::~Glyph(void) {
	FT_Bitmap_Done(ft_library,&bitmap);
}

void Arcollect::gui::font::Glyph::render(int origin_x, int origin_y, SDL_Color color) const
{
	origin_x += delta.x;
	origin_y += delta.y;
	switch (bitmap.pixel_mode) {
		case FT_PIXEL_MODE_GRAY: {
			for (unsigned int x = 0; x < bitmap.width; x++)
				for (unsigned int y = 0; y < bitmap.rows; y++) {
					unsigned char value = bitmap.buffer[y*bitmap.pitch+x];
					renderer->SetDrawColor(color.r,color.g,color.b,(value/255.f)*(color.a/255.f)*255);
					SDL_RenderDrawPoint((SDL_Renderer*)renderer,origin_x + x, origin_y + y);
				}
		} break;
	}
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
void Arcollect::gui::font::Renderable::append_text(const std::string_view& text, SDL::Point &cursor, int wrap_width, Uint32 font_size, SDL_Color color)
{
	typedef std::codecvt<char32_t,char/*char8_t*/,std::mbstate_t> codecvt_t;
	// Convert the string in UTF-32
	std::u32string codepoints;
	codepoints.resize(text.size());
	static const codecvt_t &codecvt = std::use_facet<codecvt_t>(std::locale());
	const char/*char8_t*/* trash8;
	char32_t* utf32_end;
	std::mbstate_t codecvt_state;
	codecvt.in(codecvt_state,&*(text.begin()),&*(text.end()),trash8,&*(codepoints.begin()),&*(codepoints.end()),utf32_end);
	codepoints.resize(std::distance(codepoints.data(),utf32_end));
	return append_text(codepoints,cursor,wrap_width,font_size,color);
}
void Arcollect::gui::font::Renderable::append_text(const std::u32string_view& text, SDL::Point &cursor, int wrap_width, Uint32 font_size, SDL_Color color)
{
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
	hb_font_t *font = hb_ft_font_create_referenced(face);
	hb_ft_font_set_load_flags(font,ft_flags);
	hb_shape(font,buf,NULL,0);
	hb_font_destroy(font);
	// Prepare glyphs process
	const auto glyph_base = glyphs.size();
	unsigned int glyph_count;
	hb_glyph_info_t *glyph_infos    = hb_buffer_get_glyph_infos(buf, &glyph_count);
	hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
	glyphs.reserve(glyph_base+glyph_count);
	unsigned int glyphi_line_start = 0;
	auto clusteri_line_end = text.find(U'\n'); // To break at \n
	// Process glyphs
	for (unsigned int i = 0; i < glyph_count; i++) {
		hb_glyph_info_t &glyph_info = glyph_infos[i];
		glyph_pos[i].x_advance >>= 6;
		glyph_pos[i].y_advance >>= 6;
		// Wrap text (but not if we already started a new_line, the cluster won't fit anyway)
		if ((((cursor.x + glyph_pos[i].x_advance) > wrap_width) && (glyph_info.cluster != glyph_infos[glyphi_line_start].cluster))) {
			// Search back to a safe char to wrap
			unsigned int wrap_char;
			for (wrap_char = glyph_info.cluster; wrap_char != glyph_infos[glyphi_line_start].cluster; wrap_char--)
				if ((text[wrap_char] == U' ')||(text[wrap_char] == U'\t'))
					break;
			wrap_char++;
			// Search back to the glyph index to wrap
			unsigned int i_newline;
			for (i_newline = i; i_newline > glyph_infos[wrap_char].cluster; i_newline--);
			// Replace glyphs
			cursor.x = 0;
			for (; i_newline < i; i_newline++) {
				glyphs[glyph_base+i_newline].position.x  = cursor.x;
				glyphs[glyph_base+i_newline].position.y += font_size;
				cursor.x += glyph_pos[i_newline].x_advance;
			}
			// Move the cursor
			cursor.y += font_size;
		} else if (glyph_info.cluster >= clusteri_line_end) {
			// We are on a line break '\n'
			cursor.x = 0;
			cursor.y += font_size;
			clusteri_line_end = text.find(U'\n',clusteri_line_end+1); // Find the next line break
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
			result_size.x = std::max(result_size.x,static_cast<int>(cursor.x+glyph.delta.x+glyph.bitmap.width));
			result_size.y = std::max(result_size.y,static_cast<int>(cursor.y+glyph.delta.y+glyph.bitmap.rows));
		}
		// Update cursor
		cursor.x += glyph_pos[i].x_advance;
		cursor.y += glyph_pos[i].y_advance;
	}
	// Cleanups
	hb_buffer_destroy(buf);
}
Arcollect::gui::font::Renderable::Renderable(const Elements& elements) : Renderable(elements,2147483647)
{
}
Arcollect::gui::font::Renderable::Renderable(const Elements& elements, int wrap_width) :
	result_size{0,0}
{
	SDL::Point cursor{0,0};
	FontSize font_size = elements.initial_height;
	SDL_Color color    = elements.initial_color;
	for (const Element& element: elements) {
		switch (element.index()) {
			case ELEMENT_STRING: {
				append_text(std::get<std::string>(element),cursor,wrap_width,font_size,color);
			} break;
			case ELEMENT_STRING_VIEW: {
				append_text(std::get<std::string_view>(element),cursor,wrap_width,font_size,color);
			} break;
			case ELEMENT_FONT_SIZE: {
				font_size = std::get<FontSize>(element);
			} break;
			case ELEMENT_COLOR: {
				color = std::get<SDL_Color>(element);
			} break;
		}
	}
}
static inline Arcollect::gui::font::Elements ArcollectRenderableconstcharstarHelper(const char* text, int font_size)
{
	Arcollect::gui::font::Elements elements;
	elements << std::string_view(text);
	elements.initial_height = font_size;
	return elements;
}
Arcollect::gui::font::Renderable::Renderable(const char* text, int font_size) : Renderable(ArcollectRenderableconstcharstarHelper(text,font_size))
{
}
Arcollect::gui::font::Renderable::Renderable(const char* text, int font_size, Uint32 wrap_width) : Renderable(ArcollectRenderableconstcharstarHelper(text,font_size),wrap_width)
{
}
void Arcollect::gui::font::Renderable::render_tl(int x, int y)
{
	for (const GlyphData& glyph: glyphs)
		glyph.glyph->render(glyph.position.x + x, glyph.position.y + y, glyph.color);
}
