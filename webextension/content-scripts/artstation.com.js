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
 * This platform have a complicated dynamic HTML but a neat simple API.
 */

/** Generate a tag from ArtStation API mediums/categories/software_items entries
 * \param tag object to process
 * \return The corresponding Arcollect tag
 */
function process_artstation_special_tag(tag) {
	return {
		'id': tag.name.arcollect_tag(),
		'title': tag.name,
		'icon': tag.icon_url,
	};
}
function process_artstation_artwork_api(json) {
	/** Process tags
	 *
	 * Note that we also save as tags mediums and categories.
	 */
	let tags = json.tags.map(tag => {return{'id': tag.arcollect_tag(),'title': tag}}).concat(
		json.categories.map(process_artstation_special_tag),
		json.mediums.map(process_artstation_special_tag),
		json.software_items.map(process_artstation_special_tag),
	)
	/** Accounts
	 *
	 * It's a clear single-user mapping.
	 */
	let accounts = [{
			"id": json.user.id,
			"name": json.user.username,
			"title": json.user.full_name,
			"url": json.user.permalink,
			"desc": json.user.headline,
			"icon": json.user.large_avatar_url,
	}]
	/** Rating
	 *
	 * There's various NSFW flags in the JSON.
	 */
	let rating = (json.hide_as_adult || json.adult_content || json.admin_adult_content) ? 18 : 0;
	
	/** Extract description
	 *
	 * The HTML code is provided in the API result.
	 */
	let desc = (new DOMParser()).parseFromString(json.description_html || json.description,'text/html').firstChild.textContent;
	
	/** Extract postdate
	 *
	 * A simple text timestamp
	 */
	let postdate = Math.round(Date.parse(json.created_at)/1000) ;
	
	/** Extract artworks
	 *
	 * The API provide an assets array that we filter and map.
	 */
	let artworks = json.assets.filter(asset => asset.has_image).map(asset => {return{
		"title": asset.title || json.title,
		"desc": desc,
		"source": json.permalink+new URL(asset.image_url).pathname.slice(-1)[0],
		"rating": rating,
		"postdate": postdate,
		"data": ((asset.width >= 3840)||(asset.height >= 3840)) ? asset.image_url.replace('/large/','/4k/') : asset.image_url, // FIXME Might be defective when a side is exactly 3840px long but I am lacking of test material for now
	}})
	
	return {
		'platform': 'artstation.com',
		'artworks': artworks,
		'accounts': accounts,
		'tags': tags,
		'art_acc_links': Arcollect.simple_art_acc_links(artworks,{'account': accounts}),
		'art_tag_links': Arcollect.simple_art_tag_links(artworks,tags),
	}
}

/** Select an ArtStation artworks
 * \param json from the API
 * \param matches based on direct downloads URL (the last path component)
 * \return A function that  filter API calls
 */
function select_artstation_artwork(matches) {
	matches = new Set(matches);
	return function(json) {
		json.assets = json.assets.filter(asset => matches.has(new URL(asset.image_url).pathname.slice(-1)[0]));
		return json;
	};
}

/** Save the artwork
 * \param this "Save in Arcollect" button that received click
 */
function artstation_save_artwork()
{
	// Show that we are saving the artwork
	this.onclick = null;
	this.text = arco_i18n_saving;
	
	/** Get download URL
	 *
	 * It's simple like that really !
	 */
	let artworkLink = this.nextElementSibling.href;
	let artworkAPIMatcher = select_artstation_artwork(new URL(artworkLink).pathname.slice(-1)[0]);
	
	// Perform processing
	let saveButtonA = this;
	fetch_json("https://www.artstation.com/projects/"+window.location.pathname.split('/').slice(-1)[0]+".json"
	).then(artworkAPIMatcher).then(process_artstation_artwork_api).then(Arcollect.submit).then(function() {
		saveButtonA.text = arco_i18n_saved;
	}).catch(function(reason) {
		saveButtonA.onclick = save_artwork;
		saveButtonA.text = arco_i18n_save_retry;
		console.log(arco_i18n_save_fail+' '+reason);
		alert(arco_i18n_save_fail+' '+reason);
	});
}

/** Make the "Save in Arcollect" button
 * \param assetActions The <class="asset-actions"> element to alter.
 */
function artstation_make_save_ui(assetActions) {
	if (!assetActions.saveButtonA) {
		let saveButtonA = document.createElement("a");
		saveButtonA.text = arco_i18n_save;
		// Copy parents attributes for the style
		[...assetActions.attributes].forEach(attr => saveButtonA.setAttribute(attr.name,attr.value))
		saveButtonA.className = "btn";
		saveButtonA.onclick = artstation_save_artwork.bind(saveButtonA);
		assetActions.saveButtonA = saveButtonA;
		assetActions.insertBefore(saveButtonA,assetActions.firstElementChild);
	}
}

function trigger_artwork_page() {
	let assetActionss = document.getElementsByClassName('asset-actions')
	for (let i = 0; i < assetActionss.length; i++)
		artstation_make_save_ui(assetActionss[i])
}

// FIXME That's a bit heavyweight, be more specific like before
let wrapper_main_observer = new MutationObserver(trigger_artwork_page);
wrapper_main_observer.observe(document.getElementsByTagName('app-root') [0],{
	'childList': true,
	'subtree': true,
})
trigger_artwork_page();
