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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "arcollect-debug.hpp"
#include <cstring>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#endif

Arcollect::Debug Arcollect::debug;
Arcollect::Debug::iterator Arcollect::Debug::end_ptr;

void Arcollect::Debug::turn_on_flag(const std::string_view& flag_name)
{
	using namespace std::literals::string_view_literals;
	if (flag_name == "all"sv) {
		for (Flag &flag: *this)
			flag.on = true;
		return;
	} else for (Flag &flag: *this)
		if (flag.name == flag_name) {
			flag.on = true;
			return;
		}
	// Flag not found!
	std::cerr << "Unknown debug flag \"" << flag_name << "\"\n";
}
Arcollect::Debug::Debug(void)
{
	// Set end_ptr
	end_ptr = reinterpret_cast<Arcollect::Debug::iterator>(&reinterpret_cast<char*>(this)[sizeof(Arcollect::debug)]);
	// Read env
	const char* env_value = std::getenv("ARCOLLECT_DEBUG");
	if (env_value) {
		#ifdef _WIN32
		// Init debug console
		AllocConsole();
		freopen("CONOUT$","w",stderr);
		std::cerr.clear(); // Reset the stream to apply the redirection
		#endif
		// Tokenize the string
		const char* start = env_value;
		const char* current;
		for (current = start; *current; current++) {
			if (*current == ',') {
				if (*start != ',')
					turn_on_flag(std::string_view(start,std::distance(start,current)));
				start = current + 1;
			}
		}
		// Insert last flag if any
		if (*start)
			turn_on_flag(std::string_view(start,std::distance(start,current)));
		// Log
		std::cerr << "Arcollect debug flags:";
		for (const Flag &flag: *this)
			std::cerr << (flag.on ? "\n\tON : " : "\n\tOFF: ") << flag.name	;
		std::cerr << std::endl;
	}
}
