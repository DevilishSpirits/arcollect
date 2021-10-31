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
/* Webext-adder statement to update an account
 */
UPDATE accounts SET
		acc_icon       = ?2,
		acc_name       = ?3,
		acc_title      = ?4,
		acc_desc       = ?5,
		acc_url        = ?6,
		acc_moneyurl   = ?7,
		acc_createdate = ?8
	WHERE acc_arcoid = ?1
	RETURNING acc_arcoid
;
