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
/** \file e621.net.js
 *  \brief Content script for e621 (https://e621.net/) and his SFW mirror
 *         e926 (https://e926.net/)
 *
 * This platform have a very clean HTML ! Due to it's nature, it miss many
 * informations we would expect from other art platforms.
 */

/** Global unwrapped Danbooru object
 */
var Danbooru = window.wrappedJSObject.Danbooru;

/** Download button <div> element
 *
 * It is used to place the "Save in Arcollect" button
 */
var imageDownloadLink = document.getElementById('image-download-link');

/** Save button <div> element
 *
 * It is changed by save_artwork() to reflect saving progression
 */
var saveButtonA = null;

/** Save the artwork
 */
function save_artwork()
{
	// Show that we are saving the artwork
	saveButtonA.onclick = null;
	saveButtonA.text = 'Saving...';
	saveButtonA.className = "button btn-warn";
	saveButtonA.style = 'cursor:progress;';
	Danbooru.notice('Saving...');
	
	/** Normalize source URL
	 *
	 * e621 ignore the trailing '/' in the url. Remove it if present.
	 * Also force origin to e621.
	 */
	let source = 'https://e621.net'+window.location.pathname;
	if (source[source.length-1] == '/')
		source = source.slice(0,-1);
	
	/** Extracts tags (we are on e621...) and accounts
	 *
	 * Tags are stored in #tag-list and their name in <a class="search-tag">
	 * elements. Tag category is available as class name in the parent element.
	 *
	 * For account, we'll use artist tags.
	 *
	 * e621 doesn't have fancy titles.
	 */
	let tags = []
	let accounts = []
	let art_acc_links = []
	let art_tag_links = []
	let tagList = document.getElementById('tag-list').getElementsByClassName('search-tag');
	for (let i = 0; i < tagList.length; i++) {
		// Check tag
		let category = null;
		let tag_id = tagList[i].text.arcollect_tag();
		switch (tagList[i].parentNode.className) {
			case 'category-1': { // Artists
				accounts.push({
					'id': tag_id,
					'name': tagList[i].text,
					'url': tagList[i].href,
					'icon': 'https://e621.net/apple-touch-icon.png'
				});
				art_acc_links.push({
					'account': tag_id,
					'artwork': source,
					'link': 'account'
				});
			} continue; // We handled this tag as an account link
			case 'category-4': { // Characters
				category = 'character';
			} break;
			case 'category-5': { // Species
				category = 'species';
			} break;
		};
		tags.push({
			'id': tag_id,
			'kind': category
		});
		art_tag_links.push({
			'artwork': source,
			'tag': tag_id
		});
	}
	
	/** Extract rating
	 *
	 * e621 use a #post-rating-text with custom class to change text CSS.
	 */
	let ratingBoxClass = document.getElementById('post-rating-text').className;
	let rating = ratingBoxClass.includes('post-rating-text-safe') ? 0 : ratingBoxClass.includes('post-rating-text-questionable') ? 16 : 18;
	
	/** Extract description
	 *
	 * It's".original-artist-commentary" element under #artist-commentary.
	 * \todo it may have fancy things with don't support right-now
	 */
	let description = document.getElementById('artist-commentary');
	if (description != null) // #artist-commentary doesn't necessarily exists
		description = description.getElementsByClassName('original-artist-commentary')[0].textContent;
	
	/** Extract title
	 */
	let title;
	let pool_names = document.getElementsByClassName('pool-name')
	if (pool_names.length > 0) {
		let pool_link = document.getElementsByClassName('pool-name')[0].children[0];
		title = pool_link.text.split(' ').slice(1,-2).join(' ')+' ('+pool_link.title+')'
	} else title = document.title;
	
	// Build the JSON
	submit_json = {
		'platform': 'e621.net',
		'artworks': [{
			'title': title,
			'desc': description,
			'source': source,
			'rating': rating,
			'mimetype': arcollect_mime_by_href_ext(imageDownloadLink.children[0].href),
			'postdate': (new Date(document.querySelector('meta[itemprop=uploadDate]').content)).getTime()/1000,
			'data': imageDownloadLink.children[0].href
		}],
		'accounts': accounts,
		'tags': tags,
		'art_acc_links': art_acc_links,
		'art_tag_links': art_tag_links,
	};
	
	// Submit
	arcollect_submit(submit_json).then(function() {
		saveButtonA.text = 'Saved';
		saveButtonA.className = 'button btn-success';
		saveButtonA.style = 'cursor:default;';
		Danbooru.notice('Saved in Arcollect');
	}).catch(function(reason) {
		saveButtonA.onclick = save_artwork;
		saveButtonA.text = 'Retry to save in Arcollect';
		saveButtonA.className = 'button btn-danger';
		saveButtonA.style = 'cursor:pointer;';
		console.log('Failed to save in Arcollect ! '+reason);
		alert('Failed to save in Arcollect ! '+reason);
	});
}

/** Make the "Save in Arcollect" button
 */
function make_save_ui() {
	saveButtonA = document.createElement("a");
	saveButtonA.text = "Save in Arcollect";
	saveButtonA.className = "button btn-warn";
	saveButtonA.style = 'cursor:pointer;';
	saveButtonA.onclick = save_artwork;
	
	let saveButtondiv = document.createElement("div");
	saveButtondiv.appendChild(saveButtonA);
	
	imageDownloadLink.parentNode.insertBefore(saveButtondiv,imageDownloadLink.nextElementSibling);
}

/** e621.net document.onkeypress handler
 */
function e621_document_onkeypress(e)
{
	const blacklisted_tag = [
		'INPUT',
		'SELECT',
		'TEXTAREA',
	].includes(document.activeElement.tagName);
	if ((e.shiftKey && (e.key.toLowerCase() == 's') && !blacklisted_tag))
		save_artwork();
	else return true;
	// Handled key
	return false;
}

document.onkeypress = e621_document_onkeypress;
make_save_ui();
