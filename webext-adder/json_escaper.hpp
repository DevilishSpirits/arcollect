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
/** \file json_escaper.hpp
 *  \brief A simple JSON escaper
 *
 * This escaper escape `"`, `\` and ASCII control-chars.
 */
#include <cstdint>
#include <string>
#include <string_view>
#pragma once
namespace Arcollect {
	namespace json {
		/** Escape a JSON string
		 * \param iter The iter to iterate
		 * \param end  The end;
		 * \return If `iter != end` after the function
		 */
		std::string escape_string(const std::string_view& str) {
			#define escape_control_char
			std::string result;
			result.reserve(str.size());
			for (char chr: str) {
				switch (chr) {
					case '\\': {
						result += "\\\\";
					} break;
					case '\"': {
						result += "\\\"";
					} break;
					default:
						if (((uint8_t)chr & ~0x1f)) {
							result += chr;
						} else {
							// Escape control char
							result += "\\u00";
							uint8_t halfbyte = ((uint8_t)chr >> 4) & 0xF;
							result += (halfbyte < 0xA) ? ('0' + halfbyte) : ('a' - 0xa + halfbyte);
							halfbyte = ((uint8_t)chr >> 0) & 0xF;
							result += (halfbyte < 0xA) ? ('0' + halfbyte) : ('a' - 0xa + halfbyte);
						}
				}
			}
			return result;
		}
	}
}
