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
#include <arcollect-i18n-common.hpp>
#include <config.h>
#include <cstdlib>
#include <unordered_map>
#include <ostream>

extern const Arcollect::i18n::common default_locale;
extern std::unordered_map<Arcollect::i18n::Lang,const Arcollect::i18n::common> locales;

void generate_desktop_file(std::ostream &out);
void generate_metainfo_xml(std::ostream &out);
