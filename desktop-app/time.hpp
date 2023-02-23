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
#pragma once
#include <chrono>
namespace Arcollect {
	/** Current frame number
	 *
	 * Origin is from the program start.
	 */
	extern unsigned int frame_number;
	/** Clock for the system
	 *
	 */
	using frame_clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<Arcollect::frame_clock>;
	extern time_point frame_time;
}
