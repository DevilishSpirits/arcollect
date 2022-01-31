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
/** \file test_wtf_json_parser.cpp
 *  \brief Test the WTF JSON parser
 *
 * Read a JSON and *should* ouput an identitical JSON in the output.
 */
#include "../json_escaper.hpp"
#include "../wtf_json_parser.hpp"
#include "../wtf_json_parser-string_view.hpp"
#include <iostream>
#include <limits>
#include <vector>

int main(void)
{
	char json_file[65536];
	std::cin.read(json_file,sizeof(json_file));
	// Fail if not at EOF
	if (!std::cin.eof()) {
		std::cerr << "# EOF not reached. Increase the length of \"json_file[" << sizeof(json_file) << "];\" around line " << __LINE__ << " of " << __FILE__ << " and try again." << std::endl;
		return 1;
	}
	// To see writes in real time
	std::cout.setf(std::ios::unitbuf);
	// Maximum floating point precision
	std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);
	// Stack of containers
	enum ContainerType {
		ROOT,
		ARRAY,
		OBJECT,
	};
	std::vector<ContainerType> containers{ROOT};
	// Parse
	using namespace Arcollect::json;
	char*      iter = &json_file[0];
	char* const end = &json_file[std::cin.gcount()];
	Have have;
	while (iter != end) {
		switch (containers.back()) {
			case ROOT: {
				have = what_i_have(iter,end);
			} break;
			case ARRAY: {
				Have saved_have = have;
				have = static_cast<Have>(read_array_value(iter,end));
				if ((saved_have != Have::ARRAY)&&(have != Have::ARRAY_CLOSE))
					std::cout << ",";
			} break;
			case OBJECT: {
				Have saved_have = have;
				std::string_view key;
				have = static_cast<Have>(read_object_keyval(key,iter,end));
				if ((have != Have::OBJECT_CLOSE)&&(have != Have::WTF)&&(have != Have::EOJ)) {
					if (saved_have != Have::OBJECT)
						std::cout << ",";
					std::cout << "\"" << escape_string(key) << "\":";
				}
			} break;
		}
		switch (have) {
			case Have::STRING: {
				std::string_view string;
				if (!read_string(string,iter,end)) {
					std::cerr << "Failed to read a string." << std::endl;
					return 1;
				}
				std::cout << "\"" << escape_string(string) << "\"";
			} break;
			case Have::OBJECT: {
				std::cout << "{";
				containers.push_back(OBJECT);
			} break;
			case Have::OBJECT_CLOSE: {
				if (containers.back() != OBJECT) {
					std::cerr << "Invalid syntax or parser bug (got Have::OBJECT_CLOSE) outside an object." << std::endl;
					return 1;
				}
				std::cout << "}";
				containers.pop_back();
			} break;
			case Have::ANOTHER_ONE: {
				std::cerr << "Invalid syntax or parser bug (got Have::ANOTHER_ONE)." << std::endl;
			} return 1;
			case Have::ARRAY: {
				std::cout << "[";
				containers.push_back(ARRAY);
			} break;
			case Have::ARRAY_CLOSE: {
				if (containers.back() != ARRAY) {
					std::cerr << "Invalid syntax or parser bug (got Have::ARRAY_CLOSE) outside an array." << std::endl;
					return 1;
				}
				std::cout << "]";
				containers.pop_back();
			} break;
			case Have::EOJ: {
			} return 0;
			case Have::TRUE_LITTERALLY: {
				std::cout << "true";
			} break;
			case Have::FALSE_LITTERALLY: {
				std::cout << "false";
			} break;
			case Have::NULL_LITTERALLY: {
				std::cout << "null";
			} break;
			case Have::VALUE: {
				std::cerr << "Invalid syntax or parser bug (got Have::VALUE)." << std::endl;
			} return 1;
			case Have::NUMBER: {
				long long number;
				auto saved_iter = iter;
				if (!read_number(number,iter,end)) {
					double number;
					iter = saved_iter;
					if (!read_number(number,iter,end)) {
						std::cerr << "Failed to read a number." << std::endl;
						return 1;
					}
					std::cout << number;
				} else  std::cout << number;
			} break;
			case Have::WTF: {
				std::cerr << "Syntax error." << std::endl;
			} return 1;
		};
	}
}
