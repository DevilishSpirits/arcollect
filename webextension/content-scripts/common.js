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
/** \file common.js
 *  \brief Common header for content-scripts
 *
 * This file is effectively a JavaScript "header" that declare some useful
 * functions like arcollect_submit().
 */

/** Background script connection
 *
 * At startup, the script automatically connect to the background script
 */
var arcollect__port = browser.runtime.connect();

/** List of running transactions
 *
 * This list track of transactions to the native webext-adder made by
 * arcollect_submit().
 */
var arcollect__transactions = {};
/** Next transactions
 *
 * This integer is used to generate unique transactions id to distinguish
 * between differents transactions. It is used to generate keys for 
 * #arcollect__transactions.
 */
var arcollect__next_transaction_id = 0;

/** Download an image artwork and encode it to base64
 *
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

const arco_i18n_save       = browser.i18n.getMessage('webext_save_in_arcollect');
const arco_i18n_saving     = browser.i18n.getMessage('webext_saving_in_arcollect');
const arco_i18n_save_fail  = browser.i18n.getMessage('webext_save_in_arcollect_failed');
const arco_i18n_save_retry = browser.i18n.getMessage('webext_save_in_arcollect_retry');;
const arco_i18n_saved      = browser.i18n.getMessage('webext_saved_in_arcollect');;

/** Normalize tag id
 * \param tag The image URL
 * \return The tag in a normalized form
 *
 * Perform tag normalization, replacing everything that is not a letter or a
 * digit or a question/exclamation marks with '-' and force lowercase.
 */
function arcollect_normalize_tag(tag)
{
	return tag.match(/(\p{L}|\p{N}|\?|!)+/gu).join('-').toLowerCase();
}
String.prototype.arcollect_tag = function() {
	return arcollect_normalize_tag(this)
}

/** File extension to MIME type dictionary
 *
 * Only contain supported formats.
 */
arcollect_mime_by_ext = {
	'gif' : 'image/gif',
	'jpe' : 'image/jpeg',
	'jpg' : 'image/jpeg',
	'jpeg': 'image/jpeg',
	'png' : 'image/png',
}
/** File extension to MIME type dictionary
 * \return the MIME type or undefined if not supported by Arcollect.
 */
function arcollect_mime_by_href_ext(href)
{
	href = href.split('#')[0].split('?')[0].split('.');
	return arcollect_mime_by_ext[href[href.length-1]];
}

// Answer the associated Promise
arcollect__port.onMessage.addListener(function(msg) {
	// Get the transaction id
	// Note: The background add it's own transaction_id after our one, strip it
	let transaction_id = msg['transaction_id'].split(' ')[0];
	if (msg['success'])
		arcollect__transactions[transaction_id].resolve();
	else arcollect__transactions[transaction_id].reject(msg['reason']);
	delete arcollect__transactions[transaction_id];
});

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
			// Generate a transaction id
			let transaction_id = (arcollect__next_transaction_id++).toString();
			json_object['transaction_id'] = transaction_id;
			arcollect__transactions[transaction_id] = {
				'resolve': resolve,
				'reject': reject,
			};
			// Send message
			console.log('arcollect_submit',json_object)
			arcollect__port.postMessage(json_object);
		}
	);
}
