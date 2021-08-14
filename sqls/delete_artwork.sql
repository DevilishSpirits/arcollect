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
/* Delete an artwork given an art_artid
 *
 * This is a multi-step query where you must bind the art_artid in the first
 * for each steps.
 */
/* substep 1 */
DELETE FROM art_acc_links        WHERE art_artid  = ?;
/* substep 2 */
DELETE FROM art_tag_links        WHERE art_artid  = ?;
/* substep 3 */
DELETE FROM artworks_unsupported WHERE artu_artid = ?;
/* substep 4 */
DELETE FROM artworks             WHERE art_artid  = ?;
