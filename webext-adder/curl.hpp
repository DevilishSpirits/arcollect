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
/** \file webext-adder/curl.hpp
 *  \brief Simple CURL C++ wrapper
 */
#pragma once
#include <curl/curl.h>
namespace curl {
	struct slist {
		struct curl_slist* list = NULL;
		bool append(const char* string) {
			struct curl_slist *new_list = curl_slist_append(list,string);
			if (new_list) {
				list = new_list;
				return true;
			} else return false;
		}
		bool append_header(const std::string_view& header, const std::string_view& value) {
			std::string result;
			if (value.empty()) {
				// Empty header
				result.resize(header.size()+1);
				result.back() = ';';
			} else {
				// Normal header
				result.resize(header.size()+value.size()+2);
				result[header.size()+0] = ':';
				result[header.size()+1] = ' ';
				result.replace(header.size()+2,value.size(),value);
			}
			result.replace(0,header.size(),header);
			return append(result.c_str());
		}
		bool disable_header(const std::string_view& header) {
			std::string result(header);
			result.resize(header.size()+1);
			result.back() = ':';
			result.replace(0,header.size(),header);
			return append(result.c_str());
		}
		slist(void) = default;
		slist(const slist&) = delete;
		slist& operator=(const slist&) = delete;
		slist(slist&& other) {
			std::swap(list,other.list);
		}
		~slist(void) {
			curl_slist_free_all(list);
		}
	};
	class url {
		protected:
			CURLU *handle;
		public:
			using Part = CURLUPart;
			inline url(void) : handle(curl_url()) {}
			inline url(const url& other) : handle(curl_url_dup(other.handle)) {}
			url(url&&) = delete; // TODO Move constructor
			inline ~url(void) {
				curl_url_cleanup(handle);
			}
			inline CURLUcode set(Part part, const char *content, unsigned int flags = 0) {
				return curl_url_set(handle,part,content,flags);
			}
			inline CURLUcode get(Part what, char *&part, unsigned int flags = 0) const {
				return curl_url_get(handle,what,&part,flags);
			}
	};
}
