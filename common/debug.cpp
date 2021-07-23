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
const std::string Arcollect::debug::env = []{
	const char* env_value = std::getenv("ARCOLLECT_DEBUG");
	if (env_value)
		return std::string(env_value);
	else return std::string();
}();

const std::unordered_set<std::string_view> Arcollect::debug::flags = []{
	std::unordered_set<std::string_view> debug_flags;
	if (!Arcollect::debug::env.empty()) {
		#ifdef _WIN32
		// Init debug console
		AllocConsole();
		freopen("CONOUT$","w",stderr);
		std::cerr.clear(); // Reset the stream to apply the redirection
		#endif
		// Tokenize the string
		const char* start = Arcollect::debug::env.c_str();
		const char* current;
		for (current = start; *current; current++) {
			if (*current == ',') {
				if (*start != ',') {
					// New token
					debug_flags.emplace(start,std::distance(start,current));
				}
				start = current+1;
			}
		}
		// Insert last flag if any
		if (*start)
			debug_flags.emplace(start,std::distance(start,current));
		// Log
		std::cerr << "Arcollect debug flags:";
		for (const std::string_view &flag: debug_flags)
			std::cerr << ' ' << flag;
		std::cerr << "." << std::endl;
	}
	return debug_flags;
}();

const bool Arcollect::debug::all_flag = Arcollect::debug::is_on("all");
