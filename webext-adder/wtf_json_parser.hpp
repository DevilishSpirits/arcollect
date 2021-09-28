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
/** \file wtf_json_parser.hpp
 *  \brief A simple UTF-8 JSON parser that is WTF.
 *
 * A very efficient I think and WTF in-place JSON parser that don't give a shit
 * about common sense and expect you to don't do worse shit with him.
 *
 * More seriously it's another JSON parser that is neither SAX or DOM but rather
 * an helper, you have to implement a part of the JSON parsing logic. You ask
 * what is there and then you call another function that parse what is there or
 * you give up. It's a reversed SAX, you call back the parser instead of having
 * the parser calling-back your code. This lead to an kind of hand-made JSON
 * parser with a simpler interface than SAX and no more code, you do the
 * same amount of error checking as with a SAX parser in a more streamlined way.
 *
 * This header-only library is entirely constexpr, allocation free and doesn't
 * include any header (even standard ones).
 *
 * You just need to carry around a writeable forward iterator and his end().
 * Keep in mind that it works fine with but does not detect malicious invalid
 * input, so be sure to sanitize things by yourself!
 *
 * There is no JSON serialization altough having an escaper is 100% legit.
 * You can serialize your dataset by yourself.
 */
#pragma once
namespace Arcollect {
	namespace json {
		/** Check if this is a JSON whitespace
		 * \param this_thing that may be a whitespace
		 * \return If this_thing is a whitespace according to JSON
		 * \note '\0' is not a whitespace
		 */
		constexpr bool is_whitespace(char this_thing) {
			return (this_thing == ' ')||(this_thing == '\n')||(this_thing == '\r')||(this_thing == '\t');
		}
		/** Check if the char is valid after a value
		 * \param this_thing that may be valid
		 * \return If this_thing is valid after a value
		 */
		constexpr bool is_valid_after_value(char this_thing) {
			return is_whitespace(this_thing)||
				(this_thing == ',')||
				(this_thing == ']')||
				(this_thing == '}')||
				(this_thing == '\0');
		}
		/** Skip JSON whitespace
		 * \param iter The iter to iterate
		 * \param end  The end;
		 * \return If `iter != end` after the function
		 */
		template <typename IterT>
		constexpr bool skip_whitespace(IterT &iter, const IterT end) {
			for (;(iter != end) && is_whitespace(*iter); ++iter);
			return iter != end;
		}
		/** What you can have next
		 *
		 * This enum is everything what_i_have() can return and what you must
		 * handle.
		 */
		enum class Have: char {
			STRING           = '"',
			OBJECT           = '{',
			OBJECT_CLOSE     = '}',
			ANOTHER_ONE      = ',',
			ARRAY            = '[',
			ARRAY_CLOSE      = ']',
			/** End-Of-JSON
			 *
			 * We encountered the end of your JSON
			 */
			EOJ              = '\0',
			/** Got a 'true'
			 */
			TRUE_LITTERALLY  = 't',
			/** Got a 'false'
			 */
			FALSE_LITTERALLY = 'f',
			/** Got a 'null'
			 */
			NULL_LITTERALLY  = 'n',
			/** Got a ':' (value delimiter)
			 *
			 * \note You should not need to use this value directly and go with
			 *       read_object_keyval() instead.
			 */
			VALUE            = ':',
			/** Got a number
			 */
			NUMBER           = '0',
			/** The JSON is invalid
			 *
			 * Yell at the user, you may throw a raging exception also.
			 */
			WTF              =  1,
		};
		/** Internal function to ensure that the keyword we found is correct
		 * \return if_match if match or Have::WTF if no match
		 */
		template <typename IterT>
		constexpr Have ensure_keyword(const char* eyword, Have if_match, IterT &iter, const IterT end) {
			// Check if the keyword is correct
			for (;*eyword; ++eyword, ++iter)
				if ((iter == end) || (*iter != *eyword))
					return Have::WTF;
			// Check if we are at the end
			if (iter == end)
				return if_match;
			// Check if the syntax next there if correct
			const char next_char = *iter;
			return is_valid_after_value(next_char) ? if_match : Have::WTF;
		}
		/** Read a string
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
		constexpr IterT read_string(IterT &iter, const IterT end) {
			IterT write_head = iter;
			for  (;iter != end; ++iter, ++write_head) {
				switch (*iter) {
					case '\0': {
					} return end;
					case '"':
						++iter;
						return write_head;
					case '\\': {
						// EOJ check
						if (++iter == end)
							return end;
						// Process char
						switch (*iter) {
							case '"' :*write_head = '"' ;break;
							case '\\':*write_head = '\\';break;
							case '/' :*write_head = '/';break;
							case 'b' :*write_head = '\b';break;
							case 'f' :*write_head = '\f';break;
							case 'n' :*write_head = '\n';break;
							case 'r' :*write_head = '\r';break;
							case 't' :*write_head = '\t';break;
							case 'u' : {
								// Read the codepoint value
								uint16_t codepoint = 0;
								for (auto i = 0; i < 4; i++) {
									if (++iter == end)
										return end;
									// Parse hexa digit
									char digit = *iter;
									if (digit >= 'a')
										digit -= 'a' - 0xA;
									else if (digit >= 'A')
										digit -= 'A' - 0xA;
									else digit -= '0';
									if ((digit > 0xF)||(digit < 0))
										return end;
									// Add the digit
									codepoint <<= 4;
									codepoint += digit;
								}
								// Generate the UTF-8 sequence
								if (codepoint < 0x80) {
									// ASCII character
									*write_head = codepoint;
								} else if (codepoint < 0x8000) {
									// Two-bytes UTF-8 sequence
									*write_head   = 0b11000000 | ((codepoint >>  6) & 0b00111111);
									*++write_head = 0b10000000 | ((codepoint >>  0) & 0b00111111);
								} else {
									// Three-bytes UTF-8 sequence
									*write_head   = 0b11100000 | ((codepoint >> 12) & 0b00111111);
									*++write_head = 0b10000000 | ((codepoint >>  6) & 0b00111111);
									*++write_head = 0b10000000 | ((codepoint >>  0) & 0b00111111);
								}
							} break;
							default:return end; // Invalid escape
						}
					} break;
					default: {
						// Choke on control characters
						if (((*iter < 32)&&(*iter >= 0))||(*iter == 127))
							return end;
						else *write_head = *iter;
					} break;
				}
			}
			// -- EOJ reached --
			return end;
		}
		/** Skip a string
		 * \param iter Placed just after the `"`
		 * \return The past-the-end iterator of the string or end upon errors.
		 */
		template <typename IterT>
		constexpr bool skip_string(IterT &iter, const IterT end) {
			for  (;iter != end; ++iter) {
				// Process letter
				switch (*iter) {
					case '"': {
						++iter;
					} return true;
					case '\\': {
						// EOJ check
						if (++iter == end)
							return false;
						// Process char
						switch (*iter) {
							case '"' :break;
							case '\\':break;
							case '/' :break;
							case 'b' :break;
							case 'f' :break;
							case 'n' :break;
							case 'r' :break;
							case 't' :break;
							case 'u' : {
								// TODO Handle direct codepoint
							} break;
							default:return false; // Invalid escape
						}
					} break;
					default: {
						// Choke on control characters
						if (((*iter < 32)&&(*iter >= 0))||(*iter == 127))
							return false;
					} break;
				}
			}
			// -- EOJ reached --
			return false;
		}
		/** Read a simple natural integer
		 * \param[out] out The result location (must be initialized to 0).
		 * \return true on success.
		 *
		 * This is for internal use by read_number().
		 * Upon non natural number, this function return false but let iter on the
		 * char that is not a number, read_number() use this behavior to read
		 * floating point numbers in multiple read_simple_int calls.
		 */
		template <typename numberT, typename IterT>
		constexpr bool read_simple_natural(numberT &out, IterT &iter, const IterT end) {
			for (; iter != end; ++iter) {
				if ((*iter >= '0')&&(*iter <= '9')) {
					out *= 10;
					out += *iter-'0';
				} else return is_valid_after_value(*iter);
			} return true;
		}
		/** Skip a simple natural integer
		 * \return true on success.
		 *
		 * This is for internal use by skip_number().
		 */
		template <typename IterT>
		constexpr bool skip_simple_natural(IterT &iter, const IterT end) {
			for (; iter != end; ++iter)
				if ((*iter < '0')||(*iter > '9'))
					return is_valid_after_value(*iter);
			return true;
		}
		/** Is an integral number
		 * \param[out] out The result location.
		 * \return true on success.
		 *
		 * You are expected to call this after a function returned #Have::NUMBER and
		 * if you wanna check if the value is an integral.
		 * \note `1e+3` is considered an integral number.
		 */
		template <typename IterT>
		constexpr bool is_integral_number(IterT iter, const IterT end) {
			// Check for negative number
			if (*iter == '-')
				if (++iter == end)
					return false;
			// Read integral parts
			bool has_e = false;
			for (; iter != end; ++iter)
				if ((*iter < '0')||(*iter > '9')) {
					if (!has_e && ((*iter == 'e')||(*iter == 'E'))) {
						has_e = true;
						if (++iter == end)
							return false;
						if ((*iter == '+'))
							if (++iter == end)
								return false;
					} else if (is_valid_after_value(*iter))
						return true;
				}
			return true;
		}
		/** Read a number
		 * \param[out] out The result location.
		 * \return true on success.
		 *
		 * You are expected to call this after a function returned #Have::NUMBER.
		 * This template is safe with both floating point and integers arguments.
		 * in such case, `iter` is already well placed just after the `"`, save
		 */
		template <typename numberT, typename IterT>
		constexpr bool read_number(numberT &out, IterT &iter, const IterT end) {
			if (skip_whitespace(iter,end)) {
				out = 0;
				// Handle negation
				bool negate = *iter == '-';
				if (negate)
					if (++iter == end)
						return false;
				// Read integral part
				if (read_simple_natural(out,iter,end))
					return true;
				// Read fractional part
				if (*iter == '.') {
					if (++iter == end)
						return false;
					numberT fractional_part = 0;
					bool done = read_simple_natural(fractional_part,iter,end);
					while (fractional_part > 1)
						fractional_part /= 10;
					out += fractional_part;
					if (done)
						return true;
				}
				// Read exponent
				if ((*iter == 'e')||(*iter == 'E')) {
					if (++iter == end)
						return false;
					bool negate_eponent = *iter == '-';
					if (negate_eponent || (*iter == '+'))
						if (++iter == end)
							return false;
					unsigned int exponent = 0;
					if (!read_simple_natural(exponent,iter,end))
						return false;
					// TODO Handle 0^0
					if (negate_eponent)
						while (exponent--)
							out /= 10;
					else while (exponent--)
							out *= 10;
					// Done
					return true;
				}
			} return false;
		}
		/** Skip a number
		 * \return true on success.
		 *
		 * Efficiently skip a number you don't care about.
		 */
		template <typename IterT>
		constexpr bool skip_number(IterT &iter, const IterT end) {
			if (skip_whitespace(iter,end)) {
				// Handle negation
				bool negate = *iter == '-';
				if (negate)
					if (++iter == end)
						return false;
				// Skip integral part
				if (skip_simple_natural(iter,end))
					return true;
				// Skip fractional part
				if (*iter == '.') {
					if (++iter == end)
						return false;
					bool done = skip_simple_natural(iter,end);
					if (done)
						return true;
				}
				// Read exponent
				if ((*iter == 'e')||(*iter == 'E')) {
					if (++iter == end)
						return false;
					if ((*iter == '-') || (*iter == '+'))
						if (++iter == end)
							return false;
					if (!skip_simple_natural(iter,end))
						return false;
					// Done
					return true;
				}
			} return false;
		}
		
		template <typename IterT>
		constexpr Have what_i_have(IterT &iter, const IterT end) {
			if (skip_whitespace(iter,end)) {
				switch (*iter) {
					case '"' :++iter;return Have::STRING;
					case '{' :++iter;return Have::OBJECT;
					case '}' :++iter;return Have::OBJECT_CLOSE;
					case ',' :++iter;return Have::ANOTHER_ONE;
					case '[' :++iter;return Have::ARRAY;
					case ']' :++iter;return Have::ARRAY_CLOSE;
					case '\0':       return Have::EOJ; // To cope with NUL-terminated strings
					case 't' :++iter;return ensure_keyword("rue",Have::TRUE_LITTERALLY,iter,end);
					case 'f' :++iter;return ensure_keyword("alse",Have::FALSE_LITTERALLY,iter,end);
					case 'n' :++iter;return ensure_keyword("ull",Have::NULL_LITTERALLY,iter,end);
					case ':' :++iter;return Have::VALUE;
					case '-' :       return Have::NUMBER;
					case '0' :       return Have::NUMBER;
					case '1' :       return Have::NUMBER;
					case '2' :       return Have::NUMBER;
					case '3' :       return Have::NUMBER;
					case '4' :       return Have::NUMBER;
					case '5' :       return Have::NUMBER;
					case '6' :       return Have::NUMBER;
					case '7' :       return Have::NUMBER;
					case '8' :       return Have::NUMBER;
					case '9' :       return Have::NUMBER;
					default:return Have::WTF;
				}
			} else return Have::EOJ;
		}
		/** What you can find in an object
		 *
		 * This is what read_object_keyval() can return, it is compatible with 
		 * what you #Have with what_i_have() (<- read carefully, it's no brain
		 * friendly).
		 */
		enum class ObjHave: char {
			STRING           = static_cast<char>(Have::STRING),
			OBJECT           = static_cast<char>(Have::OBJECT),
			OBJECT_CLOSE     = static_cast<char>(Have::OBJECT_CLOSE),
			ARRAY            = static_cast<char>(Have::ARRAY),
			TRUE_LITTERALLY  = static_cast<char>(Have::TRUE_LITTERALLY),
			FALSE_LITTERALLY = static_cast<char>(Have::FALSE_LITTERALLY),
			NULL_LITTERALLY  = static_cast<char>(Have::NULL_LITTERALLY),
			NUMBER           = static_cast<char>(Have::NUMBER),
			WTF              = static_cast<char>(Have::WTF),
		};
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
		constexpr ObjHave read_object_keyval(IterT &name_start, IterT &name_end, IterT &iter, const IterT end) {
			// Handle the case when we have a comma on the road
			Have have = what_i_have(iter,end);
			if (have == Have::ANOTHER_ONE)
				have = what_i_have(iter,end);
			// Check if we are at the end of this object
			if (have == Have::OBJECT_CLOSE)
				return ObjHave::OBJECT_CLOSE;
			// Read the name
			if (have != Have::STRING)
				return ObjHave::WTF;
			name_start = iter;
			name_end = read_string(iter,end);
			if (name_end == end)
				return ObjHave::WTF;
			// Read the ':'
			if (what_i_have(iter,end) != Have::VALUE)
				return ObjHave::WTF;
			// Read the value
			have = what_i_have(iter,end);
			switch (have) {
				case Have::STRING:
				case Have::ARRAY:
				case Have::OBJECT:
				case Have::TRUE_LITTERALLY:
				case Have::FALSE_LITTERALLY:
				case Have::NULL_LITTERALLY:
				case Have::NUMBER:
					return static_cast<ObjHave>(have); // Return that we found our object object
				
				case Have::ANOTHER_ONE:
				case Have::OBJECT_CLOSE: // <- forbid '"key":}' construct
				case Have::ARRAY_CLOSE:
				case Have::EOJ:
				case Have::VALUE:
				case Have::WTF:
				default:
					return ObjHave::WTF; // JSON is not valid
			}
		}
		/** What you can find in an array
		 *
		 * This is what read_array_value() can return, it is compatible with 
		 * what you #Have with what_i_have() (<- read carefully, it's no brain
		 * friendly).
		 */
		enum class ArrHave: char {
			STRING           = static_cast<char>(Have::STRING),
			OBJECT           = static_cast<char>(Have::OBJECT),
			ARRAY            = static_cast<char>(Have::ARRAY),
			ARRAY_CLOSE      = static_cast<char>(Have::ARRAY_CLOSE),
			TRUE_LITTERALLY  = static_cast<char>(Have::TRUE_LITTERALLY),
			FALSE_LITTERALLY = static_cast<char>(Have::FALSE_LITTERALLY),
			NULL_LITTERALLY  = static_cast<char>(Have::NULL_LITTERALLY),
			NUMBER           = static_cast<char>(Have::NUMBER),
			WTF              = static_cast<char>(Have::WTF),
		};
		/** Read an array value
		 * \return The type of the value (ArrHave::STRING, ArrHave::OBJECT,
		 *         ArrHave::ARRAY,ArrHave::NUMBER),
		 *         or ArrHave::ARRAY_CLOSE if we reached the end of the array,
		 *         or ArrHave::WTF on invalid JSON.
		 *
		 * You call this after what_i_have() returned Have::OBJECT, until this
		 * function return 
		 */
		template <typename IterT>
		constexpr ArrHave read_array_value(IterT &iter, const IterT end) {
			Have have = what_i_have(iter,end);
				// Handle the case when we have a comma on the road
			if (have == Have::ANOTHER_ONE)
				have = what_i_have(iter,end);
			switch (have) {
				case Have::STRING:
				case Have::ARRAY:
				case Have::ARRAY_CLOSE:
				case Have::OBJECT:
				case Have::TRUE_LITTERALLY:
				case Have::FALSE_LITTERALLY:
				case Have::NULL_LITTERALLY:
				case Have::NUMBER:
					return static_cast<ArrHave>(have); // Return that we found our object object
				
				case Have::ANOTHER_ONE:
				case Have::OBJECT_CLOSE:
				case Have::EOJ:
				case Have::VALUE:
				case Have::WTF:
				default:
					return ArrHave::WTF; // JSON is not valid
			}
		}
		/** Array iteration helper
		 *
		 * This array return an iterator that yields #ArrHave and that you can use
		 * in a `for (ArrHave have: Array(iter,end))` once you got an #Have::ARRAY;
		 *
		 * \warning It only break upon ArrHave::ARRAY_CLOSE, you must handle
		 *          ArrHave::WTF within the loop!
		 *          This iterator is only intended for use within a `for` loop, it
		 *          is not a true C++ iterator.
		 */
		template <typename IterT>
		struct Array {
			IterT& iter;
			const IterT end_iter;
			struct iterator {
				IterT& iter;
				const IterT end;
				ArrHave have;
				/** Check if we reached Array::end()
				 * \return If the iterator is an ArrHave::ARRAY_CLOSE
				 * \warning This compare nothing, it's just a stop signal.
				 */
				constexpr bool operator!=(const iterator& right) const {
					return have != ArrHave::ARRAY_CLOSE;
				}
				constexpr iterator& operator++(void) {
					have = read_array_value(iter,end);
					return *this;
				}
				constexpr ArrHave operator*(void) const {
					return have;
				}
			};
			constexpr iterator begin() {
				return ++iterator{iter,end_iter};
			}
			constexpr iterator end() {
				return iterator{iter,end_iter,ArrHave::ARRAY_CLOSE};
			}
			constexpr Array(IterT& iter, const IterT end_iter) : iter(iter), end_iter(end_iter) {}
		};
		template <typename IterT>
		constexpr bool skip_array(IterT &iter, const IterT end);
		template <typename IterT>
		constexpr bool skip_object(IterT &iter, const IterT end);
		/** Skip a value you don't care about
		 * \return true on success
		 *
		 * Skip an object you ignore
		 * \note Have::OBJECT_CLOSE and Have::ARRAY_CLOSE return false
		 */
		template <typename IterT>
		constexpr bool skip_value(Have have, IterT &iter, const IterT end)
		{
			switch (have) {
				case Have::STRING:return skip_string(iter,end);
				case Have::OBJECT:return skip_object(iter,end);
				case Have::ARRAY :return skip_array(iter,end);
				case Have::NUMBER:return skip_number(iter,end);
				
				case Have::TRUE_LITTERALLY:
				case Have::FALSE_LITTERALLY:
				case Have::NULL_LITTERALLY:
				case Have::EOJ:
					return true;
				
				default:return false;
			}
		}
		/** Skip an object
		 * \return true on success
		 *
		 * Skip an object you ignore
		 */
		template <typename IterT>
		constexpr bool skip_object(IterT &iter, const IterT end)
		{
			Have have;
			do {
				have = what_i_have(iter,end);
				// Check for object close
				if (have == Have::OBJECT_CLOSE)
					return true;
				// Handle the case when we have a comma on the road
				if (have == Have::ANOTHER_ONE)
					have = what_i_have(iter,end);
				// Skip name and ':'
				if ((have != Have::STRING)||!skip_string(iter,end)||(what_i_have(iter,end) != Have::VALUE))
					return false;
				// See what I have
				have = what_i_have(iter,end);
			} while (skip_value(have,iter,end));
			return false;
		}
		/** Skip an array
		 * \return true on success
		 *
		 * Skip an object you ignore
		 */
		template <typename IterT>
		constexpr bool skip_array(IterT &iter, const IterT end)
		{
			Have have;
			do {
				// Handle the case when we have a comma on the road
				have = what_i_have(iter,end);
				if (have == Have::ANOTHER_ONE)
					have = what_i_have(iter,end);
			} while ((have != Have::OBJECT_CLOSE)&& skip_value(have,iter,end));
			return have == Have::ARRAY_CLOSE;
		}
	}
}
