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
#include <array>
#include <climits>
#include <string_view>
namespace Arcollect {
	namespace i18n {
		/** Two-chars lang code
		 *
		 * This structure store a lang or country code like 'fr' or 'US'. It store
		 * it inside an integer so you can switch this way `case Code("fr"):`.
		 */
		struct Code {
			using int_type = int; // TODO Find a 2 chars type
			int_type value;
			constexpr operator int_type(void) const {
				return value;
			}
			constexpr Code(int_type new_value) : value(new_value) {}
			constexpr Code(void) : Code(0) {}
			constexpr Code(const char* string) : value(0) {
				value |= string[1];
				value <<= CHAR_BIT;
				value |= string[0];
			}
			std::string_view to_string(void) const {
				return std::string_view(reinterpret_cast<const char*>(&value),2);
			}
			constexpr bool operator==(const Code& other) const {
				return value == other.value;
			}
		};
		struct Lang {
			Code lang;
			Code country;
			constexpr Lang(const Code& new_lang, const Code& new_country = Code(0)) : lang(new_lang), country(new_country) {}
			constexpr Lang(const char* new_lang) : lang(new_lang) {
				if (new_lang[2])
					country = Code(new_lang+3);
			}
			constexpr bool operator==(const Lang& other) const {
				return (lang == other.lang)&&(country == other.country);
			}
		};
	}
}
namespace std {
	template <>
	struct hash<Arcollect::i18n::Code> {
		std::size_t operator()(const Arcollect::i18n::Code& value) const {
			return std::hash<Arcollect::i18n::Code::int_type>()(value.value);
		}
	};
	template <>
	struct hash<Arcollect::i18n::Lang> {
		std::size_t operator()(const Arcollect::i18n::Lang& value) const {
			return std::hash<Arcollect::i18n::Code::int_type>()(value.lang.value^value.country.value);
		}
	};
}
