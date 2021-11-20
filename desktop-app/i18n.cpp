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
#include "i18n.hpp"
#include "sdl2-hpp/SDL.hpp"

template <typename T>
static T make_translation(const SDL_Locale *prefered_locales) {
	using Arcollect::i18n::Code;
	T translations;
	if (prefered_locales)
		for (const SDL_Locale *locale = prefered_locales; locale->language; ++locale)
			translations.apply_locale(locale->language,locale->country ? Code(locale->country) : Code(0));
	return translations;
}

Arcollect::i18n::common Arcollect::i18n_common;
Arcollect::i18n::desktop_app Arcollect::i18n_desktop_app;

void Arcollect::set_locale_system(void)
{
	SDL_Locale *prefered_locales = SDL_GetPreferredLocales();
	Arcollect::i18n_common = make_translation<Arcollect::i18n::common>(prefered_locales);
	Arcollect::i18n_desktop_app = make_translation<Arcollect::i18n::desktop_app>(prefered_locales);
	SDL_free(prefered_locales);
}
