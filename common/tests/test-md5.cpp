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
#include "../md5.hpp"
#include <iostream>
#include <utility>
#include <string_view>

static constexpr std::pair<std::string_view,std::string_view> md5s[] = {
	{"d41d8cd98f00b204e9800998ecf8427e",""},
	{"eb23eb3c09fd2f247707a12a3cd56924","Arcollect"},
	{"af3995d227935bb09514a7e2420679ff","dragon <3"},
};

static constexpr auto md5_n = sizeof(md5s)/sizeof(md5s)[0];

int main(void)
{
	std::cout << "1.." << md5_n << std::endl;
	unsigned int i = 0;
	for (const std::pair<std::string_view,std::string_view> &md5: md5s) {
		if (MD5_CTX::DIGEST::from_string(md5.first) != MD5_CTX::hash(md5.second))
			std::cout << "not ";
		std::cout << "ok " << ++i << " - " << md5.first << " = md5(\"" << md5.second << "\")" << std::endl;
	}
}
