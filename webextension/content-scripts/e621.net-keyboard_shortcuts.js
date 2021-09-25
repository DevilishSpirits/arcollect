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
/** \file e621-keyboard_shortcuts.net.js
 *  \brief Content script for e621 shortcut page
 *         (https://e621.net/static/keyboard_shortcuts)
 *
 * To document Arcollect behavior in e621 interface.
 */
let h1s = document.getElementById('a-keyboard-shortcuts').getElementsByTagName('h1');
for (let i = 0; i < h1s.length; i++)
	if (h1s[i].textContent == 'Post') {
		let new_shortcut_entry = document.createElement("li");
		new_shortcut_entry.innerHTML = '<kbd class="key">shift</kbd>+<kbd class="key">s</kbd> Save in Arcollect'
		h1s[i].parentNode.querySelector('ul').appendChild(new_shortcut_entry)
		break;
	}
