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
#include "sqlite3-hpp/sqlite3.hpp"
#include <optional>
namespace Arcollect {
	namespace db {
		/** Comics support
		 * Page numbers are stored as integer in SQLite, aka signed 64 bits words.
		 * This word is divided in 3 sub numbers `0[part]4[pageno]36[subartwork]64`
		 * where :
		 * the `part` separate front/back covers, prologue, epilogue and the main story.
		 * The `pageno` is naturally the page number in this part.
		 * The `subartwork` is used for illustrated books like 0 the text and >=1 for
		 * subsequent illustrations in the page.
		 */
		namespace comics {
			constexpr sqlite3_int64 shift_partno(sqlite3_int64 number) {
				return (number & 0x0F) << 60;
			}
			/**
			 *
			 */
			enum class Part: sqlite3_int64 {
				/** No numbering
				 * 
				 * The dummy numbering is opaque and platform dependant. It is used
				 * where ordering make sense but no page number. It is used on Twitter 
				 * to group tweets, `pageno` is then the truncated tweet timestamp minus
				 * the root one's to reasonably avoid integer overflows.
				 */
				dummy       = shift_partno(-8),
				/** Front cover
				 *
				 * Page numbering start from 0, aka the page outside.
				 */
				front_cover = shift_partno(-7),
				//          = shift_partno(-6),
				//          = shift_partno(-5),
				//          = shift_partno(-4),
				//          = shift_partno(-3),
				//          = shift_partno(-2),
				/** Prologue
				 *
				 * Numbering start from 1.
				 */
				prologue    = shift_partno(-1),
				/** Main story
				 *
				 * Numbering start from 1.
				 */
				main        = shift_partno( 0),
				/** Epilogue
				 *
				 * Numbering start from 1.
				 */
				epilogue    = shift_partno(+1),
				//          = shift_partno(+2),
				//          = shift_partno(+3),
				//          = shift_partno(+4),
				//          = shift_partno(+5),
				/** Back cover
				 *
				 * Numbering is negated, the external back cover is -1, all bits sets
				 * that is sorted last by SQLite.
				 */
				back_cover  = shift_partno(+6),
				/** Bonus
				 *
				 * Numbering start from 1.
				 */
				bonus       = shift_partno(+7),
			};
			constexpr std::optional<Part> part_from_string(const std::string_view &str) {
				for (const auto& candidate: std::initializer_list<std::pair<std::string_view,Part>>{
						{"dummy"      ,Part::dummy},
						{"front_cover",Part::front_cover},
						{"prologue"   ,Part::prologue},
						{"main"       ,Part::main},
						{"epilogue"   ,Part::epilogue},
						{"bonus"      ,Part::bonus}
					}) if (candidate.first == str)
						return candidate.second;
				return std::nullopt;
			}
			constexpr auto pageno_shift = 28;
		}
	}
}
