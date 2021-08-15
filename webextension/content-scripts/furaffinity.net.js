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
/** \file furaffinity.net.js
 *  \brief Content script for FurAffinity (https://www.furaffinity.net/)
 *
 * This platform is easy to parse.
 *
 * \todo Extensive error checkings
 *
 * \todo Don't download artwork types we don't support
 */

/** Save button <div> element
 *
 * It is changed by save_artwork() to reflect saving progression
 */
var save_buttondiv = null;

function findElementByText(collection, text)
{
	for (let i = 0; i < collection.length; i++)
		if (collection[i].textContent == text)
			return collection[i]
	return null;
}


/** Get the #submissionImg
 *
 * This element allow me to get the title (the "alt") and a thumbnail.
 */
let submissionImg = document.getElementById('submissionImg');
/** Extract artwork
 *
 * To ensure maximum resolution, we search for the <a>Download</a> button that
 * link to the highest available artwork.
 */
let artworkLink = undefined;
let artworkMIME = undefined;
let downloadButtons = document.getElementsByClassName("button standard mobile-fix")
for (i = 0; i < downloadButtons.length; i++)
	if (downloadButtons[i].text == 'Download') {
		let ext = downloadButtons[i].href.split('#')[0].split('?')[0].split('.')
		ext = ext[ext.length-1].toLowerCase()
		// FurAffinity enforce UTF-8 character encoding
		if (ext == 'txt')
			artworkMIME = 'text/plain; charset=utf-8'; 
		else artworkMIME = arcollect_mime_by_ext[ext];
		if (artworkMIME != undefined) {
			artworkLink = downloadButtons[i].href;
			break;
		}
	}
/** Save the artwork
 */
function save_artwork()
{
	// Show that we are saving the artwork
	save_buttondiv.onclick = null;
	save_buttondiv.text = 'Saving...';
	
	let highlights = document.getElementsByClassName('highlight');
	/** Normalize source URL
	 *
	 * FurAffinity ignore the trailing '/' in the url. Add it if missing.
	 */
	let source = window.location.origin+window.location.pathname;
	if (source[source.length-1] != '/')
		source += '/';
	
	/** Extract the account
	 *
	 * The avatar is directly available as an <img class="submission-user-icon floatleft avatar">.
	 *
	 * To get the account name, it is the first (and unique) link under the .submission-id-sub-container element.
	 *
	 * \todo Extract "indesc" accounts
	 */
	let avatarImg = document.getElementsByClassName('submission-user-icon floatleft avatar')[0];
	let accountElement = document.getElementsByClassName('submission-id-sub-container')[0].getElementsByTagName("a")[0];
	let accountName = accountElement.text;
	let accountId = accountName.toLowerCase();
	let accountLink = accountElement.href;
	let accountJson = [{
		'id': accountId,
		'name': accountName,
		'url': accountLink,
		'icon': avatarImg.src
	}];
	let art_acc_links = [{
		'account': accountId,
		'artwork': source,
		'link': 'account'
	}];
	
	/** Extracts tags
	 *
	 * Tags are stored in <a> elements under the .tags-row element.
	 *
	 * Some tags are generated from *Category* and *Species* field.
	 */
	let tags = []
	let art_tag_links = []
	let tags_rows = document.getElementsByClassName('tags-row')[0].getElementsByTagName('a');
	for (let i = 0; i < tags_rows.length; i++) {
		tags.push({
			'id': tags_rows[i].text
		});
		art_tag_links.push({
			'artwork': source,
			'tag': tags_rows[i].text
		});
	}
	
	// Extract species
	// Also strip "(Other or general or ...)"
	let speciesTitle = findElementByText(highlights,'Species').nextElementSibling.innerText.split(' (')[0];
	// Don't save species tag for 'Unspecified / Any' species
	if (speciesTitle.split(' ')[0] != 'Unspecified') {
		let speciesTag = speciesTitle.arcollect_tag();
		tags.push({
			'id': speciesTag,
			'title': speciesTitle,
			'kind': "species"
		});
		art_tag_links.push({
			'artwork': source,
			'tag': speciesTag
		});
	}
	
	// Extract gender
	let genderTitle = findElementByText(highlights,'Gender').nextElementSibling.innerText.split(' (')[0];
	if ((genderTitle.split(' ')[0] != 'Any')&&(genderTitle.split(' ')[0] != 'Multiple')&&(genderTitle.split(' ')[0] != 'Other')) {
		let genderTag = genderTitle.toLowerCase().replaceAll(/(\W|\(|\))/gi, '-')
		tags.push({
			'id': genderTag,
			'title': genderTitle
		});
		art_tag_links.push({
			'artwork': source,
			'tag': genderTag
		});
	}
	
	/** Extract rating
	 *
	 * FurAffinity use a .rating-box with custom class to change text CSS.
	 */
	let ratingBoxClass = document.getElementsByClassName('rating-box')[0].className;
	let rating = ratingBoxClass.includes('adult') ? 18 : ratingBoxClass.includes('mature') ? 16 : 0;
	
	/** Extract description
	 *
	 * It's simply the ".submission-description .user-submitted-links" element.
	 * \todo it may have fancy things with don't support right-now
	 */
	let description = document.getElementsByClassName('submission-description user-submitted-links')[0].textContent;
	
	// Build the JSON
	submit_json = {
		'platform': 'furaffinity.net',
		'artworks': [{
			'title': submissionImg.alt,
			'desc': description,
			'source': source,
			'rating': rating,
			'mimetype': artworkMIME,
			'data': artworkLink
		}],
		'accounts': accountJson,
		'tags': tags,
		'art_acc_links': art_acc_links,
		'art_tag_links': art_tag_links,
	};
	if (artworkMIME.startsWith('text/'))
		submit_json['artworks'][0]['thumbnail'] = submissionImg.src;
	console.log('arcollect_submit('+JSON.stringify(submit_json)+')')
	
	// Submit
	arcollect_submit(submit_json).then(function() {
		save_buttondiv.text = 'Saved';
	}).catch(function(reason) {
		save_buttondiv.onclick = save_artwork;
		save_buttondiv.text = 'Retry to save in Arcollect';
		console.error('Failed to save in Arcollect ! '+reason);
		alert('Failed to save in Arcollect ! '+reason);
	});
}

/** Make the "Save in Arcollect" button
 */
function make_save_ui() {
	let button_nav = document.getElementsByClassName('aligncenter auto_link hideonfull1 favorite-nav');
	if (button_nav.length == 1) {
		button_nav = button_nav[0];
		// Create our button
		save_buttondiv = document.createElement("a");
		save_buttondiv.text = "Save in Arcollect";
		save_buttondiv.className = "button standard mobile-fix";
		save_buttondiv.onclick = save_artwork;
		// Append our button in the <div>
		button_nav.append(save_buttondiv);
	} else console.log('Arcollect error ! Found '+button_nav.length+' element(s) with class "aligncenter auto_link hideonfull1 favorite-nav".');
}

if (artworkMIME != undefined)
	make_save_ui();
else console.warn('Arcollect does not support this artwork type.')
