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
/* \file content-head.js
 * \brief Common functions for content-scripts
 */

// Connect to the background script
var arcollect__port = browser.runtime.connect()

/** Download an image artwork and encode to base64
 * \param url The image URL
 * \return A promise with the Base64 string
 * \todo Use a data URL
 */
function arcollect_download_to_base64(url)
{
	return new Promise(function(resolve, reject) {
		let xhr = new window.XMLHttpRequest();
		xhr.open('GET',url,true);
		xhr.responseType = 'blob';
		xhr.addEventListener('load', function() {
			let filereader = new FileReader();
			filereader.onload = function() {
				resolve(filereader.result.split(',',2)[1]);
			};
			filereader.onerror = reject;
			filereader.readAsDataURL(xhr.response);
		});
		xhr.addEventListener('error', reject);
		xhr.send();
	});
}
/** Submit new data into the database
 *
 * \param json_object Objects to send.
 * \return A promise
 * \see Read webext-adder/README.md for the data format.
 */
function arcollect_submit(json_object)
{
	return new Promise(
		function(resolve, reject) {
			arcollect__port.postMessage(json_object);
			resolve(null);
		}
	);
}
