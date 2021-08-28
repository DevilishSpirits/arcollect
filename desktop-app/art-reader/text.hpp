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
#include <filesystem>
#include <functional>
#include "../gui/font.hpp"
namespace Arcollect {
	namespace art_reader {
		// Convenience alias
		using TextElements = Arcollect::gui::font::Elements;
		/** Text charset
		 */
		enum class Charset {
			/** Unknown charset
			 */
			UNKNOWN,
			/** UTF-8
			 *
			 * The nice encoding.
			 */
			UTF8,
			/** UTF-32
			 *
			 * It's the native encoding of the text rendering engine
			 */
			UTF32,
		};
		/** Load a text artwork from file
		 * \param path Path to the text artwork.
		 * \param mime Artwork MIME type
		 * \return The #Arcollect::gui::font::Elements
		 *
		 * This variant use the MIME type to select the correct loader.
		 */
		TextElements text(const std::filesystem::path &path, const std::string_view &mime);
		
		/** Extract charset from MIME type definition
		 * \param      mime            The MIME type
		 * \param[out] charset_name    The charset name
		 * \return The specified charset or #Charset::UNKNOWN on error
		 *
		 * If no charset is found, charset_name is left untouched.
		 *
		 * To distriguish between *not charset* and *unknown charset*, check if
		 * charset_name has been altered: pass an empty one and check if it's still
		 * empty.
		 *
		 * \todo Make the detection more robust
		 * \warning The MIME type must be in lower-case !
		 * \note The search is simplistic, complex MIME types are not understood.
		 *       Only simple things like `text/plain; charset=utf-8` works.
		 */
		Charset mime_extract_charset(const std::string_view &mime, std::string_view &charset_name);
	};
}
