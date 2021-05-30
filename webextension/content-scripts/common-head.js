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
/* Common functions for content scripts
 */

// Connect to the background script
var arcollect__port = browser.runtime.connect()

/** Download an image artwork
 * \param url The image URL
 * \return A promise with the Base64 string
 * \todo Use a data URL
 */
function arcollect_download_image(url)
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
/** Submit new data
 * \param json_object Objects to send.
 * \return A promise
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

/** Check if the artwork is already in the collection
 * \param url The artwork source url
 * \return A promise that return a boolean. True mean that the artwork has already been saved 
 * \todo Currently always return false
 */
function arcollect_has_artwork(url)
{
	return new Promise(
		function(resolve, reject) {
			resolve(false);
		}
	);
}

