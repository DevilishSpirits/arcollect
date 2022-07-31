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
static Fc::Config fc_config(FcInitLoadConfigAndFonts());

static void face_generic_destroy(void* object)
{
	delete static_cast<Arcollect::gui::font::FaceGeneric*>(object);
}

static FT_Face face_by_filename(const std::string& filename, FT_Long index) {
	static std::unordered_map<std::size_t,FT_Face> cache;
	std::size_t key = std::hash<std::string>()(filename)^std::hash<FT_Long>()(index);
	auto iter = cache.find(key);
	if (iter == cache.end()) {
		FT_Face face;
		FT_New_Face(Arcollect::gui::font::ft_library,filename.c_str(),index,&face);
		face->generic.data = new Arcollect::gui::font::FaceGeneric();
		face->generic.finalizer = face_generic_destroy;
		iter = cache.emplace(key,face).first;
	}
	return iter->second;
}

struct face_size_entry {
	FT_Face face;
	FT_Size size;
	Fc::Pattern pattern;
	face_size_entry(const std::string& filename, FT_Long index, const Arcollect::gui::font::Renderable::RenderingState& state) :
		face(face_by_filename(filename,index))
	{
		FT_Reference_Face(face);
		FT_New_Size(face,&size);
		FT_Activate_Size(size);
		FT_Set_Pixel_Sizes(face,state.font_height,state.font_height);
	}
	~face_size_entry(void) {
		FT_Done_Face(face);
		FT_Done_Size(size);
	}
	FT_Face activate(void) {
		FT_Activate_Size(size);
		return face;
	}
};

static std::unordered_map<std::size_t,face_size_entry> face_size_entries;

static std::size_t hash_state(const Arcollect::gui::font::Renderable::RenderingState& state) {
	return std::hash<FT_UInt>()(state.font_height^(state.attrib_iter->weight.value << 8)^(state.font_height << 15));
}
static Fc::Pattern &lookup_pattern(const Arcollect::gui::font::Renderable::RenderingState& state, char32_t character)
{
	static std::unordered_map<std::size_t,std::vector<Fc::Pattern>> cache;
	std::vector<Fc::Pattern> &patterns = cache[hash_state(state)];
	// Lookup pattern 
	for (auto& pattern: patterns) {
		FcCharSet* charset;
		FcPatternGetCharSet(pattern,FC_CHARSET,0,&charset);
		if (FcCharSetHasChar(charset,character))
			return pattern;
	}
	
	// -- No pattern found! Lookup for one.
	FcResult res;
	Fc::CharSet charset;
	charset.AddChar(character);
	Fc::Pattern pattern(FcPatternBuild(NULL,
		FC_FAMILY,FcTypeString ,"system-ui",
		FC_COLOR ,FcTypeBool   ,false,
		FC_WEIGHT,FcTypeInteger,state.attrib_iter->weight,
	NULL));
	pattern.Add(FC_CHARSET,charset);
	pattern.Add(FC_PIXEL_SIZE,static_cast<int>(state.font_height));
	FcDefaultSubstitute(pattern);
	fc_config.Substitute(pattern,FcMatchPattern);
	patterns.emplace_back(FcFontMatch(fc_config,pattern,&res));
	return patterns.back();
}

void Arcollect::gui::font::os_init(void)
{
}

struct Arcollect::gui::font::shape_data {
	Fc::Pattern pattern;
};

FT_Face Arcollect::gui::font::shape_hb_buffer(const Arcollect::gui::font::Renderable::RenderingState& state, hb_buffer_t* buf, Arcollect::gui::font::shape_data* data)
{
	Fc::Pattern &pattern = data->pattern;
	// Read config
	FcChar8 *fc_filename;
	int face_index;
	FcPatternGetString(pattern,FC_FILE,0,&fc_filename);
	FcPatternGetInteger(pattern,FC_INDEX,0,&face_index);
	std::string filename(reinterpret_cast<const char*>(fc_filename));
	// Cast ray in cache
	std::size_t key = std::hash<decltype(filename)>()(filename) ^ hash_state(state) ^ face_index;
	auto iter = face_size_entries.try_emplace(key,filename,face_index,state).first;
	// Setup face
	FT_Face face = iter->second.activate();
	Arcollect::gui::font::FaceGeneric &generic = *static_cast<Arcollect::gui::font::FaceGeneric*>(face->generic.data);
	generic.hash_key = key;
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
	// Cleanups
	delete data;
	return face;
}
int Arcollect::gui::font::text_run_length(const Arcollect::gui::font::Renderable::RenderingState &state, unsigned int cp_offset, Arcollect::gui::font::shape_data*& data)
{
	decltype(cp_offset) start_offset = cp_offset;
	auto     current_attrib_iter = state.attrib_iter;
	do {
		const char32_t chr = state.text[start_offset];
		// Check if we're not on a blank
		if (!((chr <= 0x20)
			 ||(chr == 0x00A0)))
			break;
		// Loop and check for overflow
		if (++start_offset >= state.text.size()) {
			// FIXME This is a dirty workaround
			data = new Arcollect::gui::font::shape_data{
				lookup_pattern(state,U'A'),
			};
			return state.text.size()-cp_offset;
		}
		if (current_attrib_iter->end <= start_offset)
			++current_attrib_iter;
	} while (true);
	
	FontSize current_font_size = current_attrib_iter->font_size;
	Weight current_weight = current_attrib_iter->weight;
	// Allocate data and invoke FontConfig
	if (current_attrib_iter->end <= start_offset)
		++current_attrib_iter;
	data = new Arcollect::gui::font::shape_data{
		lookup_pattern(state,state.text[start_offset]),
	};
	FcCharSet* charset;
	FcPatternGetCharSet(data->pattern,FC_CHARSET,0,&charset);
	FcPatternReference(data->pattern);
	auto hash = FcPatternHash(data->pattern);
	// Find where we should break
	for (auto chr_i = start_offset + 1; chr_i < state.text.size(); ++chr_i) {
		const char32_t chr = state.text[chr_i];
		// Check for attrib iters
		if (current_attrib_iter->end < chr_i) {
			++current_attrib_iter;
			// Break on..
			if ((current_font_size != current_attrib_iter->font_size) // Size changes
			  ||(current_weight    != current_attrib_iter->weight   ) // Weight changes
				) {
				return chr_i - cp_offset - 1;
			}
		}
		// Skip blank codepoints
		if ((chr <= 0x20)
			 ||(chr == 0x00A0))
			continue;
		// -- No known pattern -> Lookup one
		if (hash != FcPatternHash(lookup_pattern(state,chr)))
			return chr_i - cp_offset;
	}
	// We processed all text
	return state.text.size()-cp_offset;
}
