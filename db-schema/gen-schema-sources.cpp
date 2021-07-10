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
/* Usage: gen-schema-sources.cpp db-schema.cpp arcollect-db-schema.hpp [current.sql...]
 *
 * This is a simple helper that is not designed to work on every edge cases, I
 * took harmless liberties here. Do not report bugs unless his behavior cause
 
 */
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>

static std::fstream cpp;
static std::fstream hpp;

static void write_sql_file(const char* sql_file_path, const std::string &var_name)
{
	std::cerr << "Including \"" << sql_file_path << "\" as Arcollect::db::schema::" << var_name << std::endl;
	char sql_file[8192];
	std::ifstream sql(sql_file_path);
	//sql.exceptions(sql.failbit|sql.badbit);
	sql.read(sql_file,sizeof(sql_file));
	// Fail if not at EOF
	if (!sql.eof()) {
		std::cerr << "EOF not reached after loading \"" << sql_file_path << "\". Increase the length of \"sql_file[" << sizeof(sql_file) << "];\" around line " << __LINE__ << " of " << __FILE__ << " and try again." << std::endl;
		std::exit(1);
	}
	// Start C++ variable
	hpp << "\t\t\textern const std::string " << var_name << ";\n";
	cpp << "const std::string Arcollect::db::schema::" << var_name << " = \"";
	// Write and minify the file
	const auto sql_size = sql.gcount();
	bool wasblank = true;
	enum {
		COMMENT_NO,     // We are not in a comment
		COMMENT_BEGIN, // We read a '/', maybe the start of a comment
		COMMENT_IN,   // We are in a comment
		COMMENT_END, // We read a '*', maybe the end of the comment
	} comment_state = COMMENT_NO;
	// case: on blanks chars
	#define case_blank case ' ':case '\t':case '\n':case '\r'
	for (auto i = 0; i < sql_size; i++)
		switch (comment_state) {
			case COMMENT_BEGIN: {
				// We may be an a comment start
				if (sql_file[i] == '*') {
					comment_state = COMMENT_IN;
					continue;
				} else cpp << '/'; // Not a comment, echo the suspended '/'
			} // falltrough;
			case COMMENT_NO: {
				switch (sql_file[i]) {
					case '/': {
						// Maybe the start of a comment
						wasblank = false;
						comment_state = COMMENT_BEGIN;
					} break;
					case_blank: {
						// Got a blank char, we collapses blanks
						if (!wasblank) {
							cpp << ' ';
							wasblank = true;
							comment_state = COMMENT_NO;
						}
					} break;
					default: {
						// Normal char
						cpp << sql_file[i];
						wasblank = false;
						comment_state = COMMENT_NO;
					}
				}
			} break;
			case COMMENT_IN: {
				// Check for comment end
				if (sql_file[i] == '*')
					comment_state = COMMENT_END;
			} break;
			case COMMENT_END: {
				// Check for comment end
				if (sql_file[i] == '/')
					comment_state = COMMENT_NO;
				else comment_state = COMMENT_IN;
			} break;
		}
	// Finish the C++
	cpp << "\";\n";
}

int main(int argc, char** argv)
{
	// Create files
	cpp.open(argv[1],std::fstream::out);
	hpp.open(argv[2],std::fstream::out);
	cpp << "/* C++ source automatically generated by " << __FILE__ << " */\n"
	"#include <arcollect-db-schema.hpp>" << std::endl;
	hpp << "/* C++ header automatically generated by " << __FILE__ << " */\n"
	"#pragma once\n#include <string>\n"
	"namespace Arcollect {\n\tnamespace db {\n\t\tnamespace schema {"
	<< std::endl;
	// Write SQL files
	for (auto i = 3; argv[i]; i++) {
		// File names are ../../dir/dir/var_name.sql
		int var_name_length = -1;
		int pos;
		for (pos = std::strlen(argv[i])-4; (pos > 0) && (argv[i][pos] != '/'); pos--)
			var_name_length++;
		std::string var_name(argv[i],pos+1,var_name_length);
		write_sql_file(argv[i],var_name);
	}
	// Finish the header
	hpp << "\t\t}\n\t}\n}\n";
}
