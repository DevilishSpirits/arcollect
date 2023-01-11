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

/** Save the artwork
 */
function save_artwork()
{
	// Show that we are saving the artwork
	saveButton.onclick = null;
	saveButton.innerText = arco_i18n_saving;
	
	let documentDivs = document.getElementsByTagName('div');
	
	/** Extract artwork
	 *
	 * Artwork is the img under a <div data-hook="art_stage">
	 */
	let art_stage = document.querySelector('div[data-hook=art_stage]');
	let artworkImg = art_stage.getElementsByTagName('img')[0];
	
	/** Try to extract the best artwork
	 *
	 * artworkData is a promise
	 */
	// Fallback to the image
	let artworkData = Promise.resolve(artworkImg);
	
	// Use the download button if available
	let download_button = document.querySelector('a[data-hook=download_button]');
	if (download_button && typeof(download_button.href) == 'string') {
		// Check the access token timestamp
		let token_expires = parseInt(new URL(download_button.href).searchParams.get('ts'));
		// Note: token_expires might be a NaN if ts is missing, in such case the
		// comparison is false and the flow continue. This is wanted behavior as it
		// allows the extension to possi0bly works if the platform change.
		if (token_expires < Date.now()/1000)
			artworkData = Promise.reject(browser.i18n.getMessage('webext_access_token_expired_pls_refresh'));
		else artworkData = Promise.resolve(Arcollect.makeDownloadSpec(download_button,{
				'redirection_count': 1,
				'cookies': true,
			}));
	}
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
	let deviationMeta = document.querySelector('div[data-hook=deviation_meta]');
	let userElement = deviationMeta.getElementsByTagName("a")[0];
	let userId = userElement.attributes['data-useruuid'].value;
	let accountJson = [{
		'id': userId,
		'name': userElement.attributes['data-username'].value,
		'url': userElement.href,
		'icon': userElement.attributes['data-icon'].value
	}];
	 
	/** Extracts tags
	 *
	 * Tags are stored in <a> elements we filter by the URL. Note that 2 forms of
	 * URLs are used depending on weather you are logged in or not.
	 */
	let tags_url_prefixes = [
		'https://www.deviantart.com/tag/',
		'https://www.deviantart.com/search/deviations?q=',
	];
	let tags = [...document.getElementsByTagName('a')].map(function(a) {
		let prefix = tags_url_prefixes.find(prefix => a.href.startsWith(prefix));
		if (prefix)
			return {
				'id': a.href.slice(prefix.length)
			};
		else return false;
	}).filter(tag => tag)
	
	/** Extract description
	 *
	 * This is the first ".legacy-journal" element.
	 */
	let description = document.getElementsByClassName('legacy-journal')[0].textContent;
	
	// Submit
	artworkData.then(function(download_spec) {
		let artworks = [{
			'title': artworkImg.alt,
			'desc': description,
			'source': source,
			// TODO 'rating': rating,
			'postdate': new Date(deviationMeta.parentElement.getElementsByTagName('time')[0].dateTime).getTime(),
			'data': download_spec,
		}];
		return {
			'platform': 'deviantart.com',
			'artworks': artworks,
			'accounts': accountJson,
			'tags': tags,
			'art_acc_links': Arcollect.simple_art_acc_links(artworks,{'account': accountJson}),
			'art_tag_links': Arcollect.simple_art_tag_links(artworks,tags),
		};
	}).then(Arcollect.submit).then(function() {
		saveButton.innerText = arco_i18n_saved;
	}).catch(function(reason) {
		saveButton.onclick = save_artwork;
		saveButton.innerText = arco_i18n_save_retry;
		console.log(arco_i18n_save_fail+' '+reason);
		alert(arco_i18n_save_fail+' '+reason);
	});
}

/** Make the "Save in Arcollect" button
 *
 * It is placed right to "Add favourites"
 */
function make_save_ui() {
	let fave_button = document.querySelector('button[data-hook=fave_button]')
	// Create the top-level button
	saveButton = document.createElement("button"); 
	// Copy styles for aesthetics
	saveButton.className = fave_button.className;
	saveButton.innerText = arco_i18n_save;
	saveButton.onclick   = save_artwork
	// Prepend the button
	fave_button.parentNode.insertBefore(saveButton,fave_button);
}

// TODO Check if we support this kind of button
make_save_ui();
