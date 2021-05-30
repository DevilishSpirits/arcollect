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

Arcollect::config::Param<int> Arcollect::config::first_run(0);
Arcollect::config::Param<int> Arcollect::config::start_window_mode(Arcollect::config::STARTWINDOW_MAXIMIZED);
Arcollect::config::Param<int> Arcollect::config::current_rating(Arcollect::config::RATING_ADULT); // FIXME Adult default is not a sane default for everyone

void Arcollect::config::read_config(void)
{
	INIReader reader(Arcollect::path::xdg_config_home/"arcollect.ini");
	// TODO if (reader.ParseError() < 0)
	first_run.value = reader.GetInteger("arcollect","first_run",first_run.default_value);
	start_window_mode.value = reader.GetInteger("arcollect","start_window_mode",start_window_mode.default_value);
	current_rating.value = reader.GetInteger("arcollect","current_rating",current_rating.default_value);
	
	// Save back updated file
	write_config();
}
void Arcollect::config::write_config(void)
{
	std::ofstream writer(Arcollect::path::xdg_config_home/"arcollect.ini");
	writer << "[arcollect]\n"
	       << "; Arcollect configuration file\n"
	       << "; This file contain Arcollect configuration\n"
	       << "; Your library is stored at " << Arcollect::path::arco_data_home << "\n"
	       << "; Note: This file is rewritten upon each start and your comments will be lost\n"
	       << "\n"
	       << "; first_run - Wheather you did the first run for this version\n"
	       << "first_run=" << first_run << '\n'
	       << "; start_window_mode - Window starting mode\n"
	       << "; " << STARTWINDOW_NORMAL     << ": Normal windowed mode\n"
	       << "; " << STARTWINDOW_MAXIMIZED  << ": Maximized window\n"
	       << "; " << STARTWINDOW_FULLSCREEN << ": Full-screen\n"
	       << "; Default is " << start_window_mode.default_value << "\n"
	       << "start_window_mode=" << start_window_mode << '\n'
	       << "\n"
	       << "; current_rating - Current artwork rating option\n"
	       << "; " << RATING_NONE   << ": Unrated only\n"
	       << "; " << RATING_PG13   << ": Not for young child (PG13)\n"
	       << "; " << RATING_MATURE << ": Mature content\n"
	       << "; " << RATING_ADULT  << ": Adult content\n"
	       << "; This option is set when you change current rating\n"
	       << "current_rating=" << current_rating << '\n'
	;
}
