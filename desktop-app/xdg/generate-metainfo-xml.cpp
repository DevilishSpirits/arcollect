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

static void write_locale_element(std::ostream &out, const std::string_view& indent, const std::string_view& key, const std::string_view& (*gettext)(const Arcollect::i18n::common&))
{
	out << indent << '<' << key << '>' << gettext(default_locale) << "</" << key << ">\n";
	for (const auto &translation: locales) {
		// Check if the translatino is worth including
		const std::string_view& value = gettext(translation.second);
		if (translation.first.country) {
			auto iter = locales.find(Arcollect::i18n::Lang(translation.first.lang));
			if ((iter != locales.end())
				&&(value == gettext(iter->second))
			) continue;
		}
		// Include translation
		out << indent << '<' << key << " xml:lang=\"" << translation.first.lang.to_string();
		if (translation.first.country)
			out << '_' << translation.first.country.to_string();
		out << "\">" << value << "</" << key << ">\n";
	}
}

#define WRITE_LOCALE_ELEMENT(indent,key,field) write_locale_element(out,indent,key,[](const Arcollect::i18n::common& locale) -> const std::string_view& { return locale.field; });

void generate_metainfo_xml(std::ostream &out)
{
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    << "<component type=\"desktop-application\">\n"
	    << "	<id>" ARCOLLECT_DBUS_NAME_STR "</id>\n"
	    << "	<name>Arcollect</name>\n";
	    WRITE_LOCALE_ELEMENT("\t","summary",summary)
	out << "	<metadata_license>CC-BY-SA-4.0</metadata_license>\n"
	    << "	<project_license>GPL-3.0-or-later</project_license>\n"
	    << "	<categories>\n"
	    << "		" << std::getenv("APPSTREAM_categories") << "\n"
	    << "	</categories>\n"
	    << "	<url type=\"homepage\">" ARCOLLECT_WEBSITE_STR "</url>\n"
	    << "	<developer_name>DevilishSpirits</developer_name>\n"
	    << "	<provides>\n"
	    << "		<dbus type=\"session\">" ARCOLLECT_DBUS_NAME_STR "</dbus>\n"
	    << "		<binary>arcollect</binary>\n"
	    << "	</provides>\n"
	    << "	<supports>\n"
	    << "		<id version=\"" ARCOLLECT_WEBEXT_GECKO_MIN_VERSION_STR "\" compare=\"ge\">org.mozilla.firefox</id>\n"
	    << "	</supports>\n"
	    << "	<description>\n"
	    << "		<p>You may want to save some artworks you find on the net. Right-click and save picture works but is not very convenient, forget the artwork source and classification is complicated.</p>\n"
	    << "		<p>Arcollect aim to fulfill my needs of artwork collection management and to ease it&apos;s creation, browsing and growth that I do in a one click thanks to the web-extension which put a Save in Arcollect button on artwork pages. Metadata like the source, account, tags and the rating are also saved.</p>\n"
	    << "		<p>It&apos;s goal is limited to easily browse and save artworks, it won&apos;t help you to discover new artworks or make any recommendations. No one can judge what&apos;s good or not better than you.</p>\n"
	    << "		<p>Arcollect respect your privacy and will never judge you.</p>\n"
	    << "	</description>\n"
	    << "	<content_rating type=\"oars-1.0\">\n"
	    << "		<!-- Might be fully Intense if the user wants. But we don't judge that. -->\n"
	    << "		<content_attribute id=\"social-info\">mild</content_attribute>\n"
	    << "	</content_rating>\n"
	    << "	<launchable type=\"desktop-id\">" ARCOLLECT_DBUS_NAME_STR ".desktop</launchable>\n"
	    << "</component>";
}
