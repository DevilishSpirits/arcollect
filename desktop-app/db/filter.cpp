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
#include "filter.hpp"
unsigned int Arcollect::db_filter::version = 0;
std::string Arcollect::db_filter::get_sql(void)
{
	return "(art_rating <= " + std::to_string(config::current_rating) + ")";
}
void Arcollect::db_filter::set_rating(config::Rating rating)
{
	config::current_rating = rating;
	Arcollect::db_filter::version++;
}
