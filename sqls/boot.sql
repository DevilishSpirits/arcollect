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
/* This SQL is ran each time a database is open.
 *
 * It perform some pragmas.
 */
PRAGMA foreign_keys = ON; /* Enforce foreign keys */
PRAGMA journal_mode  = WAL; /* WAL is more recent */
PRAGMA secure_delete = ON; /* Increase privacy */
/* Ensure maximum durability */
PRAGMA fullfsync     = ON;
PRAGMA synchronous   = EXTRA;
