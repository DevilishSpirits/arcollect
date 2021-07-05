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
#ifdef __unix__
#include <unistd.h>
#endif
#include <iostream>
#include <arcollect-db-open.hpp>
#include <arcollect-debug.hpp>
#include <rapidjson/document.h>

std::string handle_json_dom(rapidjson::Document &json_dom);
extern const std::string user_agent;
bool debug;
std::unique_ptr<SQLite3::sqlite3> db;

int main(int argc, char *argv[])
{
	#ifdef __unix__
	if (isatty(0) == 1) {
		std::cerr <<
			"Arcollect web extension adder (" << user_agent << ")\n\n"
			"This program is used by web extensions to add new artworks.\n"
			"It works with JSON and is not intended to be used by biological entities.\n"
		<< std::endl;
	}
	// TODO Handle SIGTERM
	#endif
	// Check for debug mode
	debug = Arcollect::debug::is_on("webext-adder");
	if (debug) {
		std::cerr << "Arcollect web extension adder debugging on" << std::endl;
	}
	// Main-loop
	db = Arcollect::db::open();
	std::string json_string;
	while (std::cin.good()) {
		// Read the JSON
		uint32_t data_len;
		std::cin.read(reinterpret_cast<char*>(&data_len),sizeof(data_len));
		if (debug)
			std::cerr << "Got a JSON of " << data_len << " chars. Reading..." << std::endl;
		// Quit on empty message
		if (data_len == 0)
			return 0;
		json_string.resize(data_len);
		std::cin.read(json_string.data(),data_len);
		if (debug)
			std::cerr << "JSON is loaded. Parsing..." << std::endl;
		// Parse the JSON
		rapidjson::Document json_dom;
		if (json_dom.ParseInsitu(json_string.data()).HasParseError()) {
			// FIXME Try to give more informations
			std::cerr << "JSON parse error." << std::endl;
			return 1;
		}
		std::string transaction_result = handle_json_dom(json_dom);
		// Send transaction_result
		if (debug)
			std::cerr << transaction_result << std::endl;
		data_len = transaction_result.size();
		std::cout.write(reinterpret_cast<char*>(&data_len),sizeof(data_len));
		std::cout << transaction_result;
	}
}
