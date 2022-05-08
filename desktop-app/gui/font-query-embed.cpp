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
 *  \brief Cross-platform font query (embed a Latin-only Roboto)
 */
#include "font-internal.hpp"
#include <arcollect-roboto.hpp>
#include FT_SIZES_H
static FT_Library ft_library;
static FT_Face face;
static std::unordered_map<Uint32,FT_Size> cache;
void Arcollect::gui::font::os_init(void)
{
	FT_Init_FreeType(&ft_library);
	FT_New_Memory_Face(ft_library,(const FT_Byte*)Arcollect::Roboto::Light.data(),Arcollect::Roboto::Light.size(),0,&face);
}
FT_Face Arcollect::gui::font::query_face(Uint32 font_size)
{
	Uint32 key{font_size};
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
