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
/** \file wtf_json_parser-string_view.hpp
 *  \brief std::string_view support for a simple UTF-8 JSON parser that is WTF.
 */
#pragma once
#include "wtf_json_parser.hpp"
#include <iterator>
#include <string_view>
namespace Arcollect {
	namespace json {
		/** Read a string
		 * \param[out] out The destination
		 * \param iter Placed just after the `"`
		 * \return The past-the-end iterator of the string or end upon errors.
		 *
		 * You are expected to call this after a function returned #Have::STRING,
		 * in such case, `iter` is already well placed just after the `"`, save
		 * this iterator that is the start of your string and the result of this
		 * function with is past-the-end your string on the `"`.
		 * \warning This function process JSON escapes and overwrite the string.
		 * \todo Write an example
		 */
		template <typename IterT>
		constexpr bool read_string(std::string_view &out, IterT &iter, const IterT end) {
			char* const string_start = iter;
			char* const string_end = Arcollect::json::read_string(iter,end);
			if (string_end == end)
				return false;
			else {
				out = std::string_view(string_start,std::distance(string_start,string_end));
				return true;
			}
		}
		/** Read an object key/value pair
		 * \return The type of the value (ObjHave::STRING, ObjHave::OBJECT,
		 *         ObjHave::ARRAY,ObjHave::NUMBER),
		 *         or ObjHave::OBJECT_CLOSE if we reached the end of the object,
		 *         or ObjHave::WTF on invalid JSON.
		 *
		 * You call this after what_i_have() returned Have::OBJECT, until this
		 * function return 
		 */
		template <typename IterT>
		constexpr ObjHave read_object_keyval(std::string_view &out, IterT &iter, const IterT end) {
			IterT name_start;
			IterT name_end;
			ObjHave have = read_object_keyval(name_start,name_end,iter,end);
			out = std::string_view(name_start,std::distance(name_start,name_end));
			return have;
		}
	}
}
