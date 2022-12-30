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
/** \file test-mine-extract-charset.cpp
 *  \brief MIME charset extraction `constexpr` testing
 */
#include "../art-reader/text.hpp"
#include <iostream>
using namespace Arcollect::art_reader;
struct test_pair {
	std::string_view test;
	Charset          expected_charset;
	std::string_view expected_charset_name;
};
static constexpr test_pair expected_success[] = {
	// MIME types we support
	{"text/plain; charset=utf-8" ,Charset::UTF8 ,"utf-8" },
	{"text/plain; charset=utf-32",Charset::UTF32,"utf-32"},
	// Others
};
static std::ostream &operator<<(std::ostream &lhs, Charset rhs) {
	switch (rhs) {
		case Charset::UNKNOWN:return lhs << "UNKNOWN";
		case Charset::UTF8   :return lhs << "UTF8";
		case Charset::UTF32  :return lhs << "UTF32";
		default              :return lhs << "(wtf?)";
	}
}
int main(int argc, char *argv[])
{
	constexpr auto test_count = sizeof(expected_success)/sizeof(test_pair);
	int test_num = 1;
	int result_code = 0;
	std::cout << "TAP version 13\n1.." << test_count << std::endl;
	for (const auto& test: expected_success) {
		std::string_view result_charset_name;
		Charset result_charset = Arcollect::art_reader::mime_extract_charset(test.test,result_charset_name);
		bool charset_deducted = result_charset == test.expected_charset;
		bool charset_name_deducted = result_charset_name == test.expected_charset_name;
		if (!charset_deducted || !charset_name_deducted) {
			std::cout << "not ";
			result_code = 1;
		}
		std::cout << "ok " << test_num++ << " - " << test.test << " # "
		          << "expected charset:"      << test.expected_charset      << " found:" << result_charset
		          << ", expected charset name:\"" << test.expected_charset_name << "\" found:\"" << result_charset_name
		          << "\"" << std::endl;
	}
	return result_code;
}
