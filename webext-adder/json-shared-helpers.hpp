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
#include "wtf_json_parser-string_view.hpp"
/** \file webext-adder/json-extra-helpers.hpp
 *  \brief Extra JSON helpers for the `arcollect-webext-adder`
 */
/** A numeric or string ID
 *
 * xxx_platid can oftenly be either strings or integers. This class wrap that.
 */
struct platform_id {
	std::string_view  platid_str;
	sqlite_int64 platid_int;
	
	bool IsInt64(void) const {
		return platid_int >= 0 ;
	}
	operator sqlite_int64(void) const {
		return platid_int;
	};
	operator const std::string_view&(void) const {
		return platid_str;
	};
	
	/** Bind a SQLite stmt value
	 */
	int bind(std::unique_ptr<SQLite3::stmt> &stmt, int col) const {
		if (IsInt64())
			return stmt->bind(col,platid_int);
		else return stmt->bind(col,platid_str);
	}
	
	bool operator==(const platform_id& other) const {
		if (IsInt64() && other.IsInt64())
			return platid_int == other.platid_int;
		else if (!IsInt64() && !other.IsInt64())
			return platid_str == other.platid_str;
			// Type mismatch, convert this or other numeric value into std::string
		else if (IsInt64()) // && !other.IsInt64() implied
			return std::to_string(platid_int) == other.platid_str;
		else //if (!IsInt64() && other.IsInt64() implied
			return platid_str == std::to_string(other.platid_int);
	}
	bool empty(void) const {
		return platid_str.empty();
	}
};
namespace std {
	template<> struct hash<platform_id> {
		std::size_t operator()(const platform_id& plat) const noexcept {
			if (plat.IsInt64())
				return std::hash<sqlite_int64>{}(static_cast<sqlite_int64>(plat));
			else return std::hash<std::string_view>{}(static_cast<std::string_view>(plat));
		}
	};
}
std::ostream &operator<<(std::ostream &left, const platform_id &right) {
	if (right.IsInt64())
		left << right.platid_int;
	else left << right.platid_str;
	return left;
}

/** JSON parsing exception
 *
 * This exception add some context to help debugging.
 */
struct JSONParsingError: public std::runtime_error {
	static std::string make_near(char* iter, char* end) {
		if (std::distance(iter,end) > 8)
			end = iter + 8;
		return std::string("Near ")+std::string(iter,std::distance(iter,end));
	}
	JSONParsingError(const std::string& message, char* iter, char* end) :
		std::runtime_error(make_near(iter,end)+": "+message) {}
};

/** JSON string reading helper
 * \param have What we have
 * \param[out] out The output to set
 * \param debug_string What is read
 *
 * Function for things that expect a string. Perform JSON type checking and
 * raise an exception on failure.
 * \note It cope fine with Arcollect::json::ObjHave::NULL_LITTERALLY, it does
 *       not modify `out` in such case like if it was never defined.
 */
static void json_read_string(Arcollect::json::ObjHave have, std::string_view &out, const std::string& debug_string, char*& iter, char* const end)
{
	using namespace Arcollect::json;
	if (have == ObjHave::NULL_LITTERALLY)
		return; /* Cope fine with NULL */
	if (have != ObjHave::STRING)
		throw JSONParsingError(debug_string+" must be a string.",iter,end);
	if (!read_string(out,iter,end))
		throw JSONParsingError("Error while reading "+debug_string+" string, syntax error.",iter,end);
}
/** JSON string reading helper that zero terminate the string
 * \param have What we have
 * \param[out] out The output to set
 * \param debug_string What is read
 *
 * json_read_string() that put a `'\0'` for use with C-string only API (CURL).
 */
static void json_read_string_nul_terminate(Arcollect::json::ObjHave have, std::string_view &out, const std::string& debug_string, char*& iter, char* const end)
{
	json_read_string(have,out,debug_string,iter,end);
	*const_cast<char*>(out.data() + out.size()) = '\0';
}
/** JSON int reading helper
 * \param have What we have
 * \param debug_string What is read
 * \return The integer
 *
 * Function for things that expect an integer. Perform JSON type checking and
 * raise an exception on failure.
 */
static sqlite_int64 json_read_int(Arcollect::json::ObjHave have, const std::string& debug_string, char*& iter, char* const end)
{
	using namespace Arcollect::json;
	sqlite_int64 out;
	if ((have != ObjHave::NUMBER)|| !is_integral_number(iter,end))
		throw JSONParsingError(debug_string+" must be an integral number.",iter,end);
	if (!read_number(out,iter,end))
		throw JSONParsingError("Error while reading "+debug_string+" integral number, syntax error.",iter,end);
	return out;
}

/** JSON #platform_id reading helper
 * \param have What we have
 * \param[out] out The output to set
 * \param debug_string What is read
 *
 * Function for things that expect an integer or a string. Perform JSON type
 * checking and raise an exception on failure.
 */
static void read_platform_id(Arcollect::json::ObjHave have, platform_id &out, const std::string& debug_string, char*& iter, char* const end)
{
	using namespace Arcollect::json;
	switch (have) {
		case ObjHave::NUMBER: {
			if (!read_number<decltype(out.platid_int)>(out.platid_int,iter,end))
				throw JSONParsingError("Error while reading "+debug_string+" id as number.",iter,end);
			out.platid_str = std::to_string(out.platid_int);
		} break;
		case ObjHave::STRING: {
			out.platid_int = -1;
			if (!read_string(out.platid_str,iter,end))
				throw JSONParsingError("Error while reading "+debug_string+" id as string.",iter,end);
		} break;
		default:
			throw JSONParsingError(debug_string+" id must be an integral or a string.",iter,end);
	}
	
}

/** Object with static keys iteration helper
 * \param T The mapped key type
 *
 * This template ease parsing of JSON objects that have known key names.
 * It is initialized with a map of names to enum values (like an std::map).
 * Then you instanciate a ForEachObjectSwitch::Iteration by calling this object
 * with the iter/end pair of JSON parsing in a `for(auto& : obj(iter,end))` loop
 * and the iterator magically yields a pair of key/Arcollect::json::ObjHave you
 * can use. It automatically skip any unknown keys and also stop automatically
 * when the object is closed.
 */
template <typename T>
struct ForEachObjectSwitch {
	using map_type = std::unordered_map<std::string_view,T>;
	const map_type map;
	struct Iteration {
		char*& iter;
		char* const end_iter;
		const map_type &map;
		struct iterator {
			char*& iter;
			char* const end;
			const map_type &map;
			struct ret_type {
				T key;
				Arcollect::json::ObjHave have;
			} ret;
			iterator& operator++(void) {
				using namespace Arcollect::json;
				std::string_view key_name;
				typename map_type::const_iterator map_iter;
				// Loop until we find a key we knows
				while (1) {
					// Check what we have
					ret.have = Arcollect::json::read_object_keyval(key_name,iter,end);
					// On end or error, return
					if (ret.have == ObjHave::WTF)
						throw JSONParsingError("syntax error at object \""+std::string(key_name)+"\" value.",iter,end);
					if (ret.have == ObjHave::OBJECT_CLOSE)
						return *this;
					// Try to find which key is used
					map_iter = map.find(key_name);
					if (map_iter != map.end()) {
						ret.key = map_iter->second;
						return *this;
					} else if (!skip_value(static_cast<Have>(ret.have),iter,end))
						throw JSONParsingError("failed to skip \""+std::string(key_name)+"\", JSON syntax error.",iter,end);
				}
			}
			constexpr bool operator!=(const iterator&) const {
				return ret.have != Arcollect::json::ObjHave::OBJECT_CLOSE;
			}
			constexpr const ret_type& operator*(void) const {
				return ret;
			}
		};
		iterator begin() {
			return ++iterator{iter,end_iter,map};
		}
		iterator end() {
			return iterator{iter,end_iter,map};
		}
	};
	ForEachObjectSwitch(std::initializer_list<typename map_type::value_type> init) : map(init) {}
	Iteration operator()(char*& iter, char* const end) const {
		return Iteration{iter,end,map};
	}
};
