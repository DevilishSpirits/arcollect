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
/** \file webextension/content-scripts/xkcd.com.js
 *  \brief Content script for xkcd comics (https://xkcd.com/)
 *
 * This platform have a normal clean HTML but we use the API that contain more
 * informations such as the creation date.
 */

/** Handle the JSON of the xkcd API
 * \param json object from xkcd API
 * \return A ready to submit JSON for the webext-adder
 *
 * Yes xkcd have a public API.
 */
function handle_xkcd_0json(json) {
	return {
		'platform': 'xkcd.com',
		'artworks': [{
			'title': json.title,
			'description': json.alt,
			'source': 'https://xkcd.com/'+json.num+'/',
			'rating': 0, // xkcd is for all public
			'postdate': new Date(json.year,json.month-1,json.day),
			'data': json.img,
			'license': 'CC-BY-NC-2.5',
		}],
		// There is no 'comics' because individual images are not ordered and it'd ruin artwork shuffling.
	}
}

/** Generate the webext-adder payload
 * \return A Promise with the JSON to pass to Arcollect.submit().
 *
 * We shot into the xkcd API that contains more datas, especially when the
 * artwork has been posted.
 * We locate the API endpoint using the OpenGraph URL as base since the
 * https://xkcd.com/ base return the last comic and in the, you know, very
 * unlikely but not impossible event that someone browse onto https://xkcd.com/,
 * like the comic and decide to click the "Save in Arcollect" button AND a new
 * xkcd is posted in this interval, the artwork saved will not be the one
 * displayed and Arcollect would betray the user due to the kind of bug that
 * would typically be illustrated by a nice xkcd artwork.
 */
function XKCD_MakeWebextAdderPayload() {
	return fetch_json(new URL('info.0.json',document.querySelector('meta[property="og:url"]').content)).then(handle_xkcd_0json);
}

// Insert "Save in Arcollect" buttons
new Arcollect.SaveControlHelper([...document.getElementsByClassName('comicNav')].map((comicNav) => {
	// Locate the "Random" button
	let middleLi = comicNav.children.item(comicNav.childElementCount/2);
	// Create the <a>Save in Arcollect</a>
	let saveButton = document.createElement('a');
	// Create and insert the enclosing <li>
	let newLi = document.createElement('li');
	newLi.appendChild(saveButton);
	middleLi.after(' ',newLi);
	return saveButton;
}),XKCD_MakeWebextAdderPayload);
