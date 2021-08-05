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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
/** \file artstation.com.js
 *  \brief Content script for ArtStation (https://www.artstation.com/)
 *
 * This platform have a very clean and easy to parse HTML.
 *
 * \todo Extract rating
 */

/** Save the artwork
 * \param saveButtonA The "Save in Arcollect" button that received click
 */
function artstation_save_artwork(saveButtonA)
{
	// Show that we are saving the artwork
	saveButtonA.onclick = null;
	saveButtonA.text = 'Saving...';
	
	/** Get download URL
	 *
	 * It's simple like that really !
	 */
	let artworkLink = saveButtonA.nextElementSibling.href;
	
	/** Get source URL
	 *
	 * ArtStation enforce this format : https://www.artstation.com/artwork/XXXXXX
	 *
	 * There is multiple artworks per page. To discriminate them we add a fragment
	 * derived from the real artwork URL.
	 */
	let source = window.location.origin+window.location.pathname+
		'#'+artworkLink.split('/').slice(-1)[0].split('?')[0];
	
	/** Extract account
	 *
	 * Account in stored in a <div class="artist">. There is two elements with the
	 * same content in the page.
	 */
	let artistElement = document.getElementsByClassName('artist')[0];
	let artistImg = artistElement.getElementsByTagName('img')[0];
	let artistaHeadLine = artistElement.getElementsByClassName('headline')[0];
	let accountId = artistImg.parentNode.href.split('/')[3]; // FIXME <- This is probably broken
	let account = [{
		'id': accountId,
		'name': artistImg.alt,
		'url': artistImg.parentNode.href,
		'icon': artistImg.src.replaceAll('medium','large')
	}];
	let art_acc_links = [{
		'account': accountId,
		'artwork': source,
		'link': 'account'
	}];
	
	/** Extract tags
	 *
	 * Tags are stored in <a class="... label-tag"> elements
	 */
	let tags = []
	let art_tag_links = []
	let tagList = document.getElementsByClassName('label-tag');
	for (let i = 0; i < tagList.length; i++) {
		let tagTitle = tagList[i].text.slice(1);
		let tagId = tagTitle.arcollect_tag();
		tags.push({
			'id': tagId,
			'title': tagTitle
		});
		art_tag_links.push({
			'artwork': source,
			'tag': tagId
		});
	}
	
	/** TODO Extract rating
	 */
	let rating = 0;
	
	/** Extract title
	 *
	 * It's the unique <h1> in the page
	 */
	let title = document.getElementsByTagName('h1')[0].textContent;
	
	/** Extract description
	 *
	 * It's simply <div id="project-description">
	 */
	let description = document.getElementById('project-description').textContent;
	
	// Build the JSON
	submit_json = {
		'platform': 'artstation.com',
		'artworks': [{
			'title': title,
			'desc': description,
			'source': source,
			'rating': rating,
			'data': artworkLink
		}],
		'accounts': account,
		'tags': tags,
		'art_acc_links': art_acc_links,
		'art_tag_links': art_tag_links,
	};
	console.log('arcollect_submit('+JSON.stringify(submit_json)+')')
	
	// Submit
	arcollect_submit(submit_json).then(function() {
		saveButtonA.text = 'Saved';
	}).catch(function(reason) {
		saveButtonA.onclick = save_artwork;
		saveButtonA.text = 'Retry to save in Arcollect';
		console.log('Failed to save in Arcollect ! '+reason);
		alert('Failed to save in Arcollect ! '+reason);
	});
}

/** Make the "Save in Arcollect" button
 * \param assetActions The <class="asset-actions"> element to alter.
 */
function artstation_make_save_ui(assetActions) {
	let saveButtonA = document.createElement("a");
	saveButtonA.text = "Save in Arcollect";
	saveButtonA.className = "btn";
	saveButtonA.onclick = function(){artstation_save_artwork(saveButtonA);};
	
	assetActions.insertBefore(saveButtonA,assetActions.firstElementChild);
}

function trigger_artwork_page() {
	let assetActionss = document.getElementsByClassName('asset-actions')
	for (let i = 0; i < assetActionss.length; i++)
		artstation_make_save_ui(assetActionss[i])
}

function page_changed() {
	// TODO Check for artwork page
	let project_assets_observer = new MutationObserver(trigger_artwork_page);
	project_assets_observer.observe(document.getElementsByTagName('project-assets')[0],{
		'childList': true,
	})
}
let wrapper_main_observer = new MutationObserver(page_changed);
wrapper_main_observer.observe(document.getElementsByClassName('wrapper-main')[0],{
	'childList': true,
})
trigger_artwork_page();
