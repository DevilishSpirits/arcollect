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
#pragma once
#include <array>
#include <string_view>
#define EMBEDED_DEPENDENCIES_COUNT @EMBEDED_DEPENDENCIES_COUNT@
namespace Arcollect {
	struct Dependency {
		std::string_view name;
		std::string_view version;
		std::string_view website;
		/** String length computation helper
		 * \param by_dep Number of extra bytes per dependency
		 * \param by_version Number of extra bytes per version data
		 * \param by_website Number of extra bytes per website data
		 * 
		 * This allow to generate *about_this_embed_deps* strings at compile time.
		 */
		template <typename Container>
		static constexpr int compute_about_this_embed_deps_size(const Container& deps, int by_dep, int by_version, int by_website) {
			int result = 0;
			for (const Dependency& dep: deps)
				#define arco_dep_opt_string(field) (dep.field.size() + !dep.field.empty()*by_ ## field)
				result += by_dep + dep.name.size() + arco_dep_opt_string(version) + arco_dep_opt_string(website);
				#undef arco_dep_opt_string
			return result;
		}
	};
	static constexpr std::array<Dependency,EMBEDED_DEPENDENCIES_COUNT> embeded_dependencies{{@EMBEDED_DEPENDENCIES@}};
};

