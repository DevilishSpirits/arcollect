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
/* Multi-step query that update a dwn_id reference in the database
 *
 * It is invoked with the source dwn_id in ?1 and the new dwn_id in ?2
 * \note Some references MUST NOT be updated this way.
 */
/** Step 1
 * Update artworks references
 * Exclude the frozen ones (with art_flag0:bit0 set)
 */
UPDATE artworks SET art_dwnid = ?2 WHERE (art_dwnid = ?1) AND ((art_flag0 & 1) == 0);
/** Step 2
 * Update artworks thumbnails references
 */
UPDATE artworks SET art_thumbnail = ?2 WHERE art_thumbnail = ?1;
/** Step 3
 * Update account thumbnails references
 */
UPDATE accounts SET acc_icon = ?2 WHERE acc_icon = ?1;
