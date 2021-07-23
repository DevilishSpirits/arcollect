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
/** \file main.cpp
 *  \brief Implementation of main()
 */
#include <arcollect-db-open.hpp>
#include "config.hpp"
#include "db/db.hpp"
#include "gui/main.hpp"
#include "sdl2-hpp/SDL.hpp" // For SDL_main
#ifdef _WIN32
#include <arcollect-debug.hpp>
#include <windows.h>
#include <stdio.h>
#endif

int main(int argc, char *argv[])
{
	#undef main // Alias no longer needed
	#ifdef _WIN32
	// Init debug console
	if (!Arcollect::debug::env.empty()) {
		AllocConsole();
		freopen("CONOUT$","w",stderr);
	}
	#endif
	// WARNING! Init order is important.
	// Read config
	Arcollect::config::read_config();
	// Load the db
	Arcollect::database = Arcollect::db::open();
	// Init the GUI
	if (Arcollect::gui::init())
		return 1;
	// Run GUI main-loop
	Arcollect::gui::start(argc,argv);
	while (Arcollect::gui::main());
	Arcollect::gui::stop();
	return 0;
}
