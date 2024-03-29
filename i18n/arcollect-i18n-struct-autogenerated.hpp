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
/*
#include <arcollect-i18n.hpp>
namespace Arcollect {
	namespace i18n {
	*	struct @module@ {
*/
			@locales_apply_declarations@
			@extra_fields@
			void apply_locale(const Code &lang, const Code &country = Code(0));
			void apply_locale(const Lang &lang) {
				return apply_locale(lang.lang,lang.country);
			}
			@module@(void) { @apply_C_language@ }
			constexpr static std::array<Lang,@translations_length@> translations = {@translations_content@};
/*
		};
	}
}
*/
