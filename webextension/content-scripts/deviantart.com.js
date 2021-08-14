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
/** \file deviantart.com.js
 *  \brief Content script for DeviantArt (https://www.deviantart.com/)
 *
 * This platform use React with minified class. We can identify elements using
 * the "data-hook" HTML element.
 *
 * \todo Artwork is capped at 1024px height. Download larger res if available
 *       with an account.
 *
 * \todo This platform have a "Mature" tag for ratings. Detect and ccheck if
 *       it's more like "Adult" or "Mature".
 */

/** Save button <div> element
 *
 * It is changed by save_artwork() to reflect saving progression
 */
var saveButton = null;

/** Find an the element with a given data-hook value
 */
function findElementByDataHook(collection, data_hook)
{
	for (let i = 0; i < collection.length; i++)
		for (let j = 0; j < collection[i].attributes.length; j++)
			if ((collection[i].attributes.item(j).name == 'data-hook') && (collection[i].attributes.item(j).value == data_hook))
				return collection[i]
	return null;
}
/** Save the artwork
 */
function save_artwork()
{
	// Show that we are saving the artwork
	saveButton.onclick = null;
	saveButton.innerText = 'Saving...';
	
	let documentDivs = document.getElementsByTagName('div');
	let documentAs = document.getElementsByTagName('a');
	
	/** Extract artwork
	 *
	 * Artwork is the img under a <div data-hook="art_stage">
	 */
	let art_stage = findElementByDataHook(documentDivs,'art_stage');
	let artworkImg = art_stage.getElementsByTagName('img')[0];
	
	/** Normalize source URL
	 *
	 * DeviantArt ignore the trailing '/' in the url. Remove it if present.
	 */
	let source = window.location.origin+window.location.pathname;
	if (source[source.length-1] == '/')
		source = source.slice(0,-1);
	
	/** Extract the account
	 *
	 * Account datails are availables under both <a> under
	 * <div data-hook="deviation_meta"> element.
	 *
	 * Data are stored in "data-*" HTML attributes.
	 */
	let deviationMeta = findElementByDataHook(documentDivs,'deviation_meta');
	let userElement = deviationMeta.getElementsByTagName("a")[0];
	let userId = userElement.attributes['data-useruuid'].value;
	let accountJson = [{
		'id': userId,
		'name': userElement.attributes['data-username'].value,
		'url': userElement.href,
		'icon': userElement.attributes['data-icon'].value
	}];
	let art_acc_links = [{
		'account': userId,
		'artwork': source,
		'link': 'account'
	}];
	 
	/** Extracts tags
	 *
	 * Tags are stored in <a> elements under the .tags-row element.
	 *
	 * FurAffinity doesn't have tag kind and fancy title
	 */
	let tags = []
	let art_tag_links = []
	for (let i = 0; i < documentAs.length; i++)
		if (documentAs[i].href.startsWith('https://www.deviantart.com/tag/')) {
			let tag_name = documentAs[i].href.slice(31);
			tags.push({
				'id': tag_name
			});
			art_tag_links.push({
				'artwork': source,
				'tag': tag_name
			});
		}
	
	/** Extract description
	 *
	 * This is the first ".legacy-journal" element.
	 */
	let description = document.getElementsByClassName('legacy-journal')[0].textContent;
	
	// Build the JSON
	submit_json = {
		'platform': 'deviantart.com',
		'artworks': [{
			'title': artworkImg.alt,
			'desc': description,
			'source': source,
			// TODO 'rating': rating,
			'postdate': new Date(deviationMeta.parentElement.getElementsByTagName('time')[0].dateTime).getTime(),
			'data': artworkImg.src
		}],
		'accounts': accountJson,
		'tags': tags,
		'art_acc_links': art_acc_links,
		'art_tag_links': art_tag_links,
	};
	console.log('arcollect_submit('+JSON.stringify(submit_json)+')')
	
	// Submit
	arcollect_submit(submit_json).then(function() {
		saveButton.innerText = 'Saved';
	}).catch(function(reason) {
		saveButton.onclick = save_artwork;
		saveButton.innerText = 'Retry to save in Arcollect';
		console.log('Failed to save in Arcollect ! '+reason);
		alert('Failed to save in Arcollect ! '+reason);
	});
}

/** Make the "Save in Arcollect" button
 *
 * It is placed right to "Add favourites"
 */
function make_save_ui() {
	let fave_button = findElementByDataHook(document.getElementsByTagName("button"),'fave_button');
	// Create the top-level button
	saveButton = document.createElement("button"); 
	// Copy styles for aesthetics
	saveButton.className = fave_button.className;
	saveButton.innerText = "Save in Arcollect";
	saveButton.onclick   = save_artwork
	// Prepend the button
	fave_button.parentNode.insertBefore(saveButton,fave_button);
}

// TODO Check if we support this kind of button
make_save_ui();
