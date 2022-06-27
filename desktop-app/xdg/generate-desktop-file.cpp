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

static void write_locale_string(std::ostream &out, const std::string_view& key, const std::string_view& (*gettext)(const Arcollect::i18n::common&))
{
	out << key << "=" << gettext(default_locale) << "\n";
	for (const auto &translation: locales) {
		// Check if the translatino is worth including
		const std::string_view& value = gettext(translation.second);
		if (translation.first.country) {
			auto iter = locales.find(Arcollect::i18n::Lang(translation.first.lang));
			if ((iter != locales.end())
				&&(value == gettext(iter->second))
			) continue;
		}
		
		out << key << '[' << translation.first.lang.to_string();
		if (translation.first.country)
			out << '_' << translation.first.country.to_string();
		out << "]=" << value << "\n";
	}
}

#define WRITE_LOCALE_STRING(key,field) write_locale_string(out,key,[](const Arcollect::i18n::common& locale) -> const std::string_view& { return locale.field; });

void generate_desktop_file(std::ostream &out)
{
	out << "[Desktop Entry]\n"
	       "Type=Application\n"
	       "Version=1.1\n"
	       "Name=Arcollect\n"
	       "DBusActivatable=true\n"
	       "Exec=arcollect\n"
	       "Terminal=false\n"
	       "Categories=" << std::getenv("DESKTOP_Categories") << "\n"
	       "StartupWMClass=" ARCOLLECT_X11_WM_CLASS_STR "\n";
	WRITE_LOCALE_STRING("Comment",summary);
}
