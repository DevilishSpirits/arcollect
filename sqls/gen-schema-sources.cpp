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
/* Usage: gen-schema-sources.cpp sqls.cpp [current.sql...]
 *
 * This is a simple helper that is not designed to work on every edge cases, I
 * took harmless liberties here. Do not report bugs or "improve" unless his
 * behavior cause problems.
 */
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string_view>
#include <unordered_map>
#include <vector>

static std::fstream cpp;
static char sql_file[1048576];

static std::unordered_map<std::string_view,std::string_view> substitutions{
	{"ADDER_ACCOUNTS_COLUMNS", "acc_arcoid"},
	{"ADDER_ARTWORKS_COLUMNS", "art_artid, art_flag0, art_partof, art_pageno, art_dwnid"},
	{"ADDER_COMICS_COLUMNS"  , "com_arcoid"},
};

static void write_sql_file(const char* sql_file_path, const std::string_view &var_name)
{
	//std::cerr << "Including \"" << sql_file_path << "\" as Arcollect::db::sql::" << var_name << std::endl;
	std::ifstream sql(sql_file_path);
	//sql.exceptions(sql.failbit|sql.badbit);
	sql.read(sql_file,sizeof(sql_file));
	// Fail if not at EOF
	if (!sql.eof()) {
		std::cerr << "EOF not reached after loading \"" << sql_file_path << "\". Increase the length of \"sql_file[" << sizeof(sql_file) << "];\" around line " << __LINE__ << " of " << __FILE__ << " and try again." << std::endl;
		std::exit(1);
	}
	// Start C++ variable
	cpp << "const std::string_view Arcollect::db::sql::" << var_name << " = \"";
	// Write and minify the file
	bool needblank = false;
	bool wasblank = false;
	enum {
		NORMAL,         // We are in initial state (reading plain SQL)
		COMMENT_BEGIN, // We read a '/', maybe the start of a comment
		COMMENT_IN,   // We are in a comment
		COMMENT_END, // We read a '*', maybe the end of the comment
		STRING_IN,  // We are in a string
		SUBST,     // We are parsing a substitution ($BLA_BLA)
	} state = NORMAL;
	std::vector<std::pair<std::string_view,std::string_view::size_type>> subst_stack{
		{{&sql_file[0],static_cast<std::string_view::size_type>(sql.gcount())},0}
	};
	std::string_view::size_type i_subst_start;
	// case: on blanks chars
	#define case_blank case ' ':case '\t':case '\n':case '\r'
	while (!subst_stack.empty()) {
		const std::string_view sql_file = subst_stack.back().first;
		auto &i = subst_stack.back().second;
		if (i >= sql_file.size()) {
			subst_stack.pop_back();
			if (state == SUBST) {
				std::string_view subst = std::string_view(&sql_file[i_subst_start],i-i_subst_start);
				auto iter = substitutions.find(subst);
				if (iter == substitutions.end()) {
					std::cerr << "Unknow substitution $" << subst << " in " << sql_file_path << std::endl;
					std::exit(1);
				}
				subst_stack.emplace_back(iter->second,0);
			}
		} else switch (state) {
			case COMMENT_BEGIN: {
				// We may be an a comment start
				if (sql_file[i] == '*') {
					state = COMMENT_IN;
					continue;
				} else {
					cpp << '/'; // Not a comment, echo the suspended '/'
					wasblank = false;
					needblank = false;
				}
			} // falltrough;
			case NORMAL: {
				bool will_wasblank = false;
				switch (sql_file[i]) {
					case '\'': {
						// String start
						cpp << '\'';
						state = STRING_IN;
					} break;
					case '/': {
						// Maybe the start of a comment
						state = COMMENT_BEGIN;
					} break;
					case '$': {
						state = SUBST;
						i_subst_start = i + 1;
					} break;
					case_blank: {
						// Skip blank chars
						state = NORMAL;
						will_wasblank = true;
					} break;
					default: {
						bool will_needblank = (sql_file[i] >= 0x80) // Non ASCII character
							||((sql_file[i] >= 'a')&&(sql_file[i] <= 'z')) // [a-z]
							||((sql_file[i] >= 'A')&&(sql_file[i] <= 'Z')) // [A-Z]
							||((sql_file[i] >= '0')&&(sql_file[i] <= '9')) // [0-9]
						;
						// Normal char
						if (needblank && wasblank && will_needblank)
							cpp << ' '; // Put a blank if needed
						cpp << sql_file[i];
						state = NORMAL;
						needblank = will_needblank;
					}
				}
				wasblank  = will_wasblank;
			} break;
			case COMMENT_IN: {
				// Check for comment end
				if (sql_file[i] == '*')
					state = COMMENT_END;
			} break;
			case COMMENT_END: {
				// Check for comment end
				if (sql_file[i] == '/')
					state = NORMAL;
				else state = COMMENT_IN;
			} break;
			case STRING_IN: {
				cpp << sql_file[i];
				// Check for string end (and not an escape)
				if ((sql_file[i] == '\'')&&(sql_file[i-1] != '\\')) {
					state = NORMAL;
					wasblank = false;
					needblank = false;
				}
			} break;
			case SUBST: {
				if (!(((sql_file[i] >= 'A')&&(sql_file[i] <= 'Z'))||((sql_file[i] >= 'a')&&(sql_file[i] <= 'z'))||(sql_file[i] == '_'))) {
					std::string_view subst = std::string_view(&sql_file[i_subst_start],i-i_subst_start);
					auto iter = substitutions.find(subst);
					if (iter == substitutions.end()) {
						std::cerr << "Unknow substitution $" << subst << " in " << sql_file_path << std::endl;
						std::exit(1);
					}
					subst_stack.emplace_back(iter->second,0);
					state = NORMAL;
					cpp << ' '; // Put a blank if needed
					continue;
				}
			} break;
		}
		++i;
	}
	// Finish the C++
	cpp << "\";\n";
}

int main(int argc, char** argv)
{
	// Create files
	cpp.open(argv[1],std::fstream::out);
	cpp << "/* C++ source automatically generated by " << __FILE__ << " */\n"
	"#include <arcollect-sqls.hpp>" << std::endl;
	// Write SQL files
	for (auto i = 2; argv[i]; i++) {
		// File names are ../../dir/dir/var_name.sql
		int var_name_length = 0;
		int pos;
		for (pos = std::strlen(argv[i])-4; (pos > 0) && (argv[i][pos-1] != '/'); pos--)
			var_name_length++;
		write_sql_file(argv[i],std::string_view(&argv[i][pos],var_name_length));
	}
}
