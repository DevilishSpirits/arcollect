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
#include "generate-xdg-files.hpp"
#include <fstream>

std::unordered_map<Arcollect::i18n::Lang,const Arcollect::i18n::common> locales;
typedef void(*file_generator)(std::ostream&);
int main(int argc, char** argv)
{
	// Load locales
	for (auto locale: Arcollect::i18n::common::translations) {
		Arcollect::i18n::common new_locale;
		new_locale.apply_locale(locale);
		locales.emplace(locale,new_locale);
	}
	// Generators configuration
	
	const file_generator file_generators[] = {
		generate_desktop_file,
		generate_metainfo_xml,
	};
	// Invoke generators
	for (auto generator: file_generators) {
		std::ofstream out(*++argv);
		generator(out);
	}
}
