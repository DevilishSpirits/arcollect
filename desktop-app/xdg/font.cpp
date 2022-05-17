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
/** \file desktop-app/gui/font-query-embed.cpp
 *  \brief FontConfig specific font query code
 *  \todo Support multi charset reading
 */
#include "fontconfig.hpp"
#include "../gui/font-internal.hpp"
#include FT_SIZES_H
static FT_Face face;
static std::unordered_map<Uint32,FT_Size> cache;
static Arcollect::gui::font::FaceGeneric face_generic; // TODO Use more faces
void Arcollect::gui::font::os_init(void)
{
	// TODO Error checkings
	// Lookup system font infos
	Fc::Config::EnableHome(true);
	Fc::Config config(FcInitLoadConfigAndFonts());
	Fc::Pattern pattern(FcPatternBuild(NULL,FC_FAMILY,FcTypeString,"system-ui",NULL));
	FcDefaultSubstitute(pattern);
	config.Substitute(pattern,FcMatchPattern);
	FcResult res;
	Fc::Pattern result(FcFontMatch(config,pattern,&res));
	// Read config
	FcChar8 *filename;
	FcPatternGetString(result,FC_FILE,0,&filename);
	// Init font
	FT_New_Face(Arcollect::gui::font::ft_library,(const char*)filename,0,&face);
	face->generic.data = &face_generic;
}
FT_Face Arcollect::gui::font::shape_hb_buffer(const Arcollect::gui::font::Renderable::RenderingState& state, hb_buffer_t* buf, Arcollect::gui::font::shape_data*)
{
	// Query the font_size
	FT_UInt key{state.font_height};
	// FIXME This is a poor prehashing to avoid collisions in case of identity std::hash
	face_generic.hash_key = std::hash<decltype(key)>()(key^(key << 8)^(key << 15));
	auto iter = cache.find(key);
	if (iter == cache.end()) {
		FT_Size new_size;
		FT_New_Size(face,&new_size);
		FT_Activate_Size(new_size);
		FT_Set_Pixel_Sizes(face,state.font_height,state.font_height);
		cache.emplace(key,new_size);
	} else FT_Activate_Size(iter->second);
	// Configure the buffer
	// FIXME Auto-detect better values
	hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
	hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language(buf, hb_language_from_string("en", -1));
	// Shape the buffer
	hb_font_t *font = hb_ft_font_create_referenced(face);
	hb_ft_font_set_load_flags(font,ft_flags);
	hb_shape(font,buf,NULL,0);
	hb_font_destroy(font);
	return face;
}
int Arcollect::gui::font::text_run_length(const Arcollect::gui::font::Renderable::RenderingState &state, unsigned int cp_offset, Arcollect::gui::font::shape_data*&)
{
	auto     current_attrib_iter = state.attrib_iter;
	FontSize current_font_size = current_attrib_iter->font_size;
	// Break on text size changes
	while ((current_attrib_iter->end < state.text.size())&&(current_font_size == current_attrib_iter->font_size))
		++current_attrib_iter;
	return current_attrib_iter->end - cp_offset;
}
