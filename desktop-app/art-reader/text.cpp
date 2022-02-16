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
/** \file text.cpp
 *  \brief Common text artwork reader implementation
 *
 * This file implement basic text artworks readers. Advanced ones are stored
 * in another source files.
 */
#include "text.hpp"
#include <fstream>

Arcollect::art_reader::Charset Arcollect::art_reader::mime_extract_charset(const std::string_view &mime, std::string_view &charset_name)
{
	// Search for "charset="
	constexpr std::string_view search = "charset="sv;
	auto search_start = mime.find(search);
	if (search_start == mime.npos)
		return Charset::UNKNOWN;
	// Set charset_name
	// TODO Make it more robust
	search_start += search.size();
	charset_name = mime.substr(search_start,mime.find(' ',search_start));
	// Search
	if (charset_name == "utf-8"sv)
		return Charset::UTF8;
	else if (charset_name == "utf-32"sv)
		return Charset::UTF32;
	else if (charset_name == "us-ascii"sv)
		// UTF-8 is US-ASCII compatible
		return Charset::UTF8;
	else return Charset::UNKNOWN;
}
Arcollect::gui::font::Elements Arcollect::art_reader::text(const std::filesystem::path &path, const std::string_view &mime)
{
	constexpr SDL::Color Y{0xFFFF00ff}; // Yellow
	constexpr SDL::Color W{0xFFFFFFff}; // White
	// Load artwork
	std::string file_content;
	std::getline(std::ifstream(path),file_content,'\0');
	const std::u32string_view file_content_as_utf32(reinterpret_cast<char32_t*>(file_content.data()),file_content.size()/sizeof(char32_t));
	// Configure elements
	TextElements elements;
	// TODO elements.initial_height = ;
	// Check if we are handling 'text/' MIME type
	constexpr std::string_view mime_text_prefix = "text/"sv;
	if (mime.starts_with(mime_text_prefix)) {
		// Check charset
		std::string_view charset_name;
		Charset charset = mime_extract_charset(mime,charset_name);
		if ((charset == Charset::UNKNOWN) && !charset_name.empty()) {
			// Unsupported charset, print diagnostic
			return elements << Y << charset_name << W << U" charset is not supported ("sv << Y << mime << W << U")."sv;
		}
		// Load text
		auto subtype = mime.substr(mime_text_prefix.size(),mime.find(';',mime_text_prefix.size())-mime_text_prefix.size());
		if (subtype == "plain")
			switch (charset) {
				case Charset::UNKNOWN: // Default is US-ASCII that is UTF-8 compatible
				case Charset::UTF8:
					return elements << file_content;
				case Charset::UTF32:
					return elements << file_content_as_utf32;
			}
		else if (subtype == "rtf") {
			return text_rtf(&*file_content.begin(),&*file_content.end());
		} return elements << Y << mime << W << U" text types are not supported."sv;
	} else if (mime == "application/rtf")
		return text_rtf(&*file_content.begin(),&*file_content.end());
	return elements << Y << mime << W << U" text types are not supported."sv;
}
