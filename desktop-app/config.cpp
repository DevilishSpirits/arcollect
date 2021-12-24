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
#include "config.hpp"
#include <INIReader.h>
#include <fstream>
#include "lcms2.h"

Arcollect::config::Param<int> Arcollect::config::start_window_mode(Arcollect::config::STARTWINDOW_MAXIMIZED);
Arcollect::config::Param<int> Arcollect::config::current_rating(Arcollect::config::RATING_ADULT); // FIXME Adult default is not a sane default for everyone
Arcollect::config::Param<int> Arcollect::config::image_memory_limit(2048);
Arcollect::config::Param<int> Arcollect::config::littlecms_intent(INTENT_PERCEPTUAL);
Arcollect::config::Param<int> Arcollect::config::littlecms_flags(cmsFLAGS_HIGHRESPRECALC|cmsFLAGS_BLACKPOINTCOMPENSATION);
Arcollect::config::Param<int> Arcollect::config::writing_font_size(18);
Arcollect::config::Param<int> Arcollect::config::rows_per_screen(5);

void Arcollect::config::read_config(void)
{
	INIReader reader((Arcollect::path::xdg_config_home/"arcollect.ini").string().c_str());
	// TODO if (reader.ParseError() < 0)
	start_window_mode.value = reader.GetInteger("arcollect","start_window_mode",start_window_mode.default_value);
	current_rating.value = reader.GetInteger("arcollect","current_rating",current_rating.default_value);
	image_memory_limit.value = reader.GetInteger("arcollect","image_memory_limit",image_memory_limit.default_value);
	littlecms_flags.value = reader.GetInteger("arcollect","littlecms_flags",littlecms_flags.default_value);
	littlecms_intent.value = reader.GetInteger("arcollect","littlecms_intent",littlecms_intent.default_value);
	writing_font_size.value = reader.GetInteger("arcollect","writing_font_size",writing_font_size.default_value);
	rows_per_screen.value = reader.GetInteger("arcollect","rows_per_screen",rows_per_screen.default_value);
}
void Arcollect::config::write_config(void)
{
	std::ofstream writer(Arcollect::path::xdg_config_home/"arcollect.ini");
	writer << "[arcollect]\n"
	       << "; Arcollect configuration file\n"
	       << "; This file contain Arcollect configuration\n"
	       << "; Your library is stored at " << Arcollect::path::arco_data_home << "\n"
	       << "; Note: Your comments won't be preserved\n"
	       << "\n"
	       << "; " << STARTWINDOW_NORMAL     << ": Normal windowed mode\n"
	       << "; " << STARTWINDOW_MAXIMIZED  << ": Maximized window\n"
	       << "; " << STARTWINDOW_FULLSCREEN << ": Full-screen\n"
	       << "; Default is " << start_window_mode.default_value << "\n"
	       << "start_window_mode=" << start_window_mode << '\n'
	       << "\n"
	       << "; current_rating - Current artwork rating option\n"
	       << "; " << RATING_NONE   << ": Unrated only\n"
	       << "; " << RATING_MATURE << ": Mature content\n"
	       << "; " << RATING_ADULT  << ": Adult content\n"
	       << "; This option is set when you change current rating\n"
	       << "current_rating=" << current_rating << '\n'
	       << "\n"
	       << "; image_memory_limit - The maximum amount of memory used by images in MiB\n"
	       << "; This is a kind of VRAM limit. Arcollect will unload artworks when the size of loaded pixels exceed this value.\n"
	       << "; The consumed memory is estimated by computing the size needed to store artworks in a uncompressed form.\n"
	       << "; Default is " << image_memory_limit.default_value << " (" << image_memory_limit.default_value/1024 << " GiB)\n"
	       << "image_memory_limit=" << image_memory_limit << '\n'
	       << "\n"
	       << "; littlecms_intent - Color management rendering intent\n"
	       << "; This define the ICC rendering intent passed to cmsCreateTransform() call. Get known about color management to get the meaning of this param.\n"
	       << "; Common values :\n"
	       << ";  INTENT_PERCEPTUAL           : " << INTENT_PERCEPTUAL            << "\n"
	       << ";  INTENT_RELATIVE_COLORIMETRIC: " << INTENT_RELATIVE_COLORIMETRIC << "\n"
	       << ";  INTENT_SATURATION           : " << INTENT_SATURATION            << "\n"
	       << ";  INTENT_ABSOLUTE_COLORIMETRIC: " << INTENT_ABSOLUTE_COLORIMETRIC << "\n"
	       << ";  See LittleCMS API documentation for more values.\n"
	       << "; Default is " << littlecms_intent.default_value << "\n"
	       << "littlecms_intent=" << littlecms_intent << '\n'
	       << "\n"
	       << "; littlecms_flags - Color management flags\n"
	       << "; Thesse flags is directly passed to LittleCMS cmsCreateTransform() function.\n"
	       << "; Many useful values (combine by summing values, or 0 for none) :\n"
	       << ";  cmsFLAGS_NULLTRANSFORM          (" << cmsFLAGS_NULLTRANSFORM          << "): Skip color management.\n"
	       << ";  cmsFLAGS_HIGHRESPRECALC         (" << cmsFLAGS_HIGHRESPRECALC         << "): Use more memory to give better accurancy.\n"
	       << ";  cmsFLAGS_BLACKPOINTCOMPENSATION (" << cmsFLAGS_BLACKPOINTCOMPENSATION << "): Enable black-point compensation.\n"
	       << ";  See LittleCMS API documentation for more flags.\n"
	       << "; Default is " << littlecms_flags.default_value << "\n"
	       << "littlecms_flags=" << littlecms_flags << '\n'
	       << "\n"
	       << "; writing_font_size - Font height of textual artworks in pixels\n"
	       << "; This is the text height in pixels used to render text artworks.\n"
	       << "; Default is " << writing_font_size.default_value << "\n"
	       << "writing_font_size=" << writing_font_size << '\n'
	       << "\n"
	       << "; rows_per_screen - Number of rows per screen\n"
	       << "; This adjust the height of rows in the grid view to display the given number of rows at full screen.\n"
	       << "; Default is " << rows_per_screen.default_value << "\n"
	       << "rows_per_screen=" << rows_per_screen << '\n'
	;
}
