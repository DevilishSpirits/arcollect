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
/* Webext-adder statement to add a new artwork
 */
INSERT INTO artworks (
		art_dwnid,
		art_thumbnail,
		art_flag0,
		art_platform,
		art_title,
		art_desc,
		art_source,
		art_rating,
		art_license,
		art_partof,
		art_pageno,
		art_postdate
	) VALUES (
		?,
		?,
		0,
		?,
		?,
		?,
		?,
		?,
		?,
		?,
		?,
		?
	) RETURNING $ADDER_ARTWORKS_COLUMNS
;
