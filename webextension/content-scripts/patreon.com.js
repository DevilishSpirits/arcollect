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
/** \file webextension/content-scripts/patreon.com.js
 *  \brief Content script for Patreon (https://www.patreon.com/)
 *
 * This platform have the typical messy HTML code but data-tag attributes allows
 * to navigate in the tree.
 */

/** Make a Patreon CDN download specification
 *
 * This function currently do nothing but I expect to add things later.
 */
function make_Patreon_cdn_downspec(url) {
	return url;
}

/** Data specification generator
 *
 * It's a dictionary of Patreon postType to function that returns an array of
 * "artworks" that will be submitted to Arcollect.
 *
 * Account and tags links are automatically added. A comic is generated from
 * page 1 to n, if needed too. The only entry you need to provide is "data", the
 * rest will be automatically filled-in but you can override these.
 *
 * \note postcard.arcollect is a foreign object and the sandbox reject any
 * method call on these objects since that's foreign code, to perform a
 * array.map(f), you need to Array.map.apply(array,[f]) that use our version of
 * the map() code and avoid a "Permission denied to access object" error.
 */
const Patreon_dataByPostTypeGenerator = {
	'image_file': (postcard) => { return Array.prototype.map.apply(postcard.arcollect.images,[(img) => { return {
		"data": make_Patreon_cdn_downspec(img.downloadUrl),
	}}])},
};

/** Generate the webext-adder payload for a .postcard element
 * \param this postcard element
 * \return A Promise with the JSON to pass to Arcollect.submit().
 */
function Patreon_MakeWebextAdderPayload_postcard(props)
{
	let postcard = this;
	
	/* Extract account
	 *
	 * We have all the infos of the uploader.
	 */
	let creator = props.campaign;
	let account_id = parseInt(creator.id);
	let accounts = [{
		"id": account_id,
		"name": creator.url.split('/')[3], // This is the vanity in Patreon user endpoint we recreate there
		"title": creator.name,
		"url": creator.url,
		"desc": creator.creationName,
		"icon": make_Patreon_cdn_downspec(creator.avatarPhotoUrl),
		// TODO "createdate"
	}];
	// Query artwork defaults
	let defaultTitle = postcard.querySelector('[data-tag="post-title"]').textContent;
	let defaultDesc = postcard.querySelector('[data-tag="post-content-collapse"],[data-tag="post-content"]').textContent;
	let defaultSource = postcard.querySelector('[data-tag="post-published-at"]').href
	let defaultRating = (props.isNsfw === false) ? 0 : 18; // Seen as "undefined" on NSFW Patreons, so use extra safety there
	let defaultPostDate = props.publishedAt;
	/* Generate artworks
	 */
	let artworks = Patreon_dataByPostTypeGenerator[props.postType](postcard).map((artwork,index) => {
		if (!artwork.hasOwnProperty('title'))	
			artwork.title = defaultTitle;
		if (!artwork.hasOwnProperty('desc'))
			artwork.desc = defaultDesc;
		if (!artwork.hasOwnProperty('source'))
			artwork.source = defaultSource+'#'+index;
		if (!artwork.hasOwnProperty('rating'))
			artwork.rating = defaultRating;
		if (!artwork.hasOwnProperty('postdate'))
			artwork.postdate = defaultPostDate;
		return artwork;
	});
	
	/* Extract tags
	 */
	let tags = [...postcard.querySelectorAll('[data-tag="post-tags"] [data-tag="post-tag"]')].map(tag => {return{
		'id': tag.textContent.arcollect_tag(),
	}})
	// Create the comic if needed
	let comics = [];
	if (artworks.length > 1) {
		let comic_id = props.id
		comics = [{
			"id": comic_id,
			"title": defaultTitle,
			"url": defaultSource,
			"postdate": defaultPostDate,
			"pages": Object.fromEntries(artworks.map((artwork,index) => {return [artwork.source,{"relative_to": "main", "page": index+1, "sub": 0}]})),
		}];
	}
	// Build the JSON
	return {
		'platform': 'patreon.com',
		'artworks': artworks,
		'accounts': accounts,
		'tags': tags,
		'comics': comics,
		'art_acc_links': Arcollect.simple_art_acc_links(artworks,{'account': accounts}),
		'art_tag_links': Arcollect.simple_art_tag_links(artworks,tags),
		'com_acc_links': Arcollect.simple_com_acc_links(comics,{'account': accounts}),
		'com_tag_links': Arcollect.simple_com_tag_links(comics,tags),
	};
}

/** Make the "Save in Arcollect" button
 */
function make_Patreon_save_ui(postcard) {
	// Check if the postcard has already been configured
	if (postcard.arcollect)
		return;
	// Instrospect React
	let props = postcard.wrappedJSObject[Object.getOwnPropertyNames(postcard.wrappedJSObject).find(x => x.startsWith("__reactFiber"))].return.stateNode.props;
	if (!(props.postType in Patreon_dataByPostTypeGenerator)) {
		console.warn("Discarding ",postcard," because post type '"+props.postType+"' is unsupported");
		return; // Post type is not supported right-now
	}
	// Create the save UI
	let postDetails = postcard.querySelector('[data-tag="post-details"]');
	// Duplicate the "like" button
	let likeButton = postDetails.querySelector('button[data-tag="like-count-engagement"]');
	let saveInArcollectBtn = likeButton.cloneNode();
	saveInArcollectBtn.style = 'margin-right:1rem;';
	new Arcollect.SaveControlHelper(saveInArcollectBtn,Patreon_MakeWebextAdderPayload_postcard.bind(postcard,props));
	let likeButtonContainer = likeButton.parentElement.parentElement;
	likeButtonContainer.insertBefore(saveInArcollectBtn,likeButtonContainer.firstChild);
	// save properties in the postcard
	postcard.arcollect = props;
}

function make_Patreon_all_save_ui() {
	for (let postcard of document.querySelectorAll('[data-tag="post-card"]'))
		make_Patreon_save_ui(postcard);
}

// Trigger UI making
// TODO Watch for less mutations
new MutationObserver(make_Patreon_all_save_ui).observe(document.querySelector('#renderPageContentWrapper'),{
	'childList': true,
	'subtree': true,
});
make_Patreon_all_save_ui()
