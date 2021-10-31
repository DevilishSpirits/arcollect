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
/* Webext-adder statement to update an artwork
 */
UPDATE artworks SET
		art_dwnid     = ?2,
		art_thumbnail = ?3,
		art_title     = ?4,
		art_desc      = ?5,
		art_license   = ?6,
		art_partof    = ?7,
		art_pageno    = ?8,
		art_postdate  = ?9
	WHERE art_artid = ?1
	RETURNING art_artid, art_flag0, art_partof, art_pageno, art_dwnid
;
