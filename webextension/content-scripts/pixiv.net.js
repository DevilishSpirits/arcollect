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
/** \file pixiv.net.js
 *  \brief Pixiv main-script (https://www.pixiv.net/)
 *
 * Pixiv is a well-written, mature and fairly complex platform.
 * Working on it is a pleasureful challenge with each obstacle being technically
 * justified for security or easeness. This push in the limits of Arcollect and
 * this script require more complexity than others. Also the first ever platform
 * on which I hit the API (and I really dislike that but they forced my hand).
 *
 * The rest is handled by this file that manage the *Save in Arcollect* button
 * and user interactions.
 *
 * It also expose the following variables for you :
 * - The `source` of the current artwork to use.
 * - A handy fetch_pixiv() API wrapper that promise the body of API calls.
 * - PixivCDNDownload() that put security headers required for Pixiv.
 */
let pixiv_lang = document.querySelector('html').lang;

function fetch_pixiv(url) {
	// Tweak the URL
	url = new URL(url);
	url.searchParams.set('lang',pixiv_lang);
	// Fetch!
	return fetch_json(url,{'headers': {
		'Sec-Fetch-Dest': 'empty',
		'Sec-Fetch-Mode': 'cors',
		'Sec-Fetch-Site': 'same-origin',
	}}).then(function(json) {
		if (json.error)
			throw (json.message);
		else return json.body;
	});
}

/** Make a Pixiv download specification that hit the CDN
 *
 * I set the `Referer` to `https://www.pixiv.net/` to match Pixiv behavior.
 */
function PixivCDNDownload(url, SecFetchDest) {
	return {
		'data': url,
		'headers': {
			'Referer': 'https://www.pixiv.net/',
		},
	};
}

function makePixivAccount(id) {
	return fetch_pixiv('https://www.pixiv.net/ajax/user/'+id+'?full=1').then(function(json) {
		return {
			'id': json.userId,
			'name': json.name,
			'url': 'https://www.pixiv.net/users/'+json.userId,
			'desc': json.comment,
			'icon': PixivCDNDownload(json.imageBig,'image'),
		}
	});
};
/** Query accounts
 * \param json The JSON to submit for webext-adder.
 * \return The Promise of the JSON with accounts data.
 */
function makePixivAccounts(json) {
	return Promise.all([...new Set([
		...json.art_acc_links.map(acc => acc.account),
		...json.com_acc_links.map(acc => acc.account),
	])].map(makePixivAccount)).then(function(accounts) {
		json.accounts = accounts;
		return json;
	});
}

/** Get rating from Pixiv xRestrict
 */
function pixiv_rating(xRestrict) {
	switch (xRestrict) {
		default:
			console.warn('Unknow Pixiv "xRestrict"',xRestrict,'=> assuming adult (R18) rating');
		case 1: // R18
		case 2: // R18-G
			return 18;
		case 0: // All public
			return 0;
	}
}

/** Return a flat list of tags from the API's `"tags"` object
 * \return The array of tags, with the 
 *
 * \warning You must use this function as it also feed the #tags_cache with data
 *          required to generate the final `"tags"` array.
 */
function pivix_process_tags(tags) {
	let list = [];
	let arco_tags = [];
	for (let tag of tags.tags) {
		let tag_id = tag.tag.arcollect_tag();
		list.push(tag_id);
		// Cache the data
		let tag_name = tag.tag;
		if (tag.hasOwnProperty('translation') && tag.translation.hasOwnProperty(pixiv_lang))
			tag_name = tag.translation[pixiv_lang];
		arco_tags.push({
			'id': tag_id,
			'name': tag_name,
		});
	}
	return {
		'list': list,
		'tags': arco_tags,
	};
}

function pixiv_save_illust(id) {
	return fetch_pixiv('https://www.pixiv.net/ajax/illust/'+id).then(function(illust) {
		// Fetch pages if required
		if (illust.pageCount == 1)
			return [illust,[illust.urls.original]];
		else return fetch_pixiv('https://www.pixiv.net/ajax/illust/'+id+'/pages').then(
			pages => [illust,pages.map(page => page.urls.original)]
		);
	}).then(function(json) {
		// Generate the webext-adder JSON
		illust = json[0];
		pages = json[1];
		
		let postdate = Date.parse(illust.createDate)/1000;
		let tags = pivix_process_tags(illust.tags);
		let canonicalSource = illust.extraData.meta.alternateLanguages.ja; // TODO Find a more robust canonical URL
		let result = {
			'artworks': [],
			'art_acc_links': [],
			'art_tag_links': [],
			'comics': [{
				'id': illust.illustId,
				'title': illust.title,
				'url': canonicalSource,
				'postdate': postdate,
				'pages': {},
			}],
			'com_acc_links': [{
				'account': illust.userId,
				'comic': illust.illustId,
				'link': 'account'
			}],
			'com_tag_links': tags.list.map(tag => Object({
				'tag': tag, 'comic': illust.illustId
			})),
			'tags': tags.tags,
		}
		let artwork_skel = {
			'title': illust.title,
			'desc': illust.description,
			'rating': pixiv_rating(illust.xRestrict),
			'postdate': postdate,
		};
		for (let i = 0; i < illust.pageCount; ++i) {
			let source = canonicalSource+'#'+(i+1);
			result.artworks.push(Object.assign({
				'data': PixivCDNDownload(pages[i],'image'),
				'source': source,
			},artwork_skel));
			result.art_acc_links.push({
				'artwork': source,
				'account': illust.userId,
				'link': 'account'
			});
			tags.list.forEach(tag => result.art_tag_links.push({
				'tag': tag, 'artwork': source
			}));
			
			result.comics[0].pages[source] = {'page': i+1};
		}
		return result;
	});
}

/** Save the artwork in Arco
 *
 * Pivix support various artworks types and we rely on specialized functions
 * that Promise the webext-adder JSON with artworks, comics and links. We do
 * fetch tags informations outselfs.
 * 
 */
function pixiv_SaveInArcollect_click() {
	let btn = this;
	let span = btn.children[0];
	btn.onclick = null;
	span.textContent = arco_i18n_saving;
	btn.style = 'cursor:progress;';
	// Invoke the backend
	btn.arcollect_save_backend(btn.arcollect_save_id).then(makePixivAccounts).then(function(json){
		json.platform = 'pixiv.net';
		return json;
	}).then(Arcollect.submit).then(function() {
		// Finished!
		span.textContent = arco_i18n_saved;
		btn.style = 'cursor:default;';
	}).catch(function(reason) {
		btn.onclick = pixiv_SaveInArcollect_click;
		span.textContent = arco_i18n_save_retry;
		btn.style = ''
		console.error(arco_i18n_save_fail+' '+reason);
		alert(arco_i18n_save_fail+' '+reason);
	});
}

var SaveInArcollect_ui = null;

/* Insert the Save in Arcollect button in the bar
 *
 * Pixiv HTML code is minified like Twitter. I use the いいね <button> as both an
 * anchor and a 'Save in Arcollect' button template.
 *
 * I locate this element by looking for the three-dot menu that I first locate
 * through aria attributes to understand the page though I don't feel very confident there.
 */
function pixivMakeSaveInArcollectButton(save_backend,save_id) {
	// Locate things
	let iine = document.querySelector('main section button[aria-haspopup=true]').parentElement.parentElement.parentElement.querySelector('div > button > span').parentElement.parentElement;
	let button_bar = iine.parentElement;
	// Create and configure the node
	SaveInArcollect_ui = iine.cloneNode(true);
	SaveInArcollect_ui.querySelector('svg').remove();
	SaveInArcollect_ui.querySelector('span').textContent = arco_i18n_save;
	let SaveInArcollect_btn = SaveInArcollect_ui.children[0];
	SaveInArcollect_btn.onclick = pixiv_SaveInArcollect_click;
	SaveInArcollect_btn.arcollect_save_backend = save_backend;
	SaveInArcollect_btn.arcollect_save_id = save_id;
	// Insert the node
	button_bar.insertBefore(SaveInArcollect_ui,iine);
}
/* Make the UI to insert the Save in Arcollect button
 *
 * This function does nothing if the button is still visible.<
 */
function pixivMakeSaveInArcollectUI() {
	// Check if we need to remake the UI
	if (SaveInArcollect_ui && (SaveInArcollect_ui.getRootNode() == document.getRootNode()))
		// The node exist and is still in the document DOM. Do nothing.
		return;
	// Query infos about the artwork to save
	let canonical_link = document.head.querySelector('link[rel=canonical]').href.split('/');
	// Get the ID
	let save_id = canonical_link[canonical_link.length-1];
	if (save_id == '')
		return;
	// Set the save backend
	let save_backend = null;
	switch (canonical_link[canonical_link.length-2]) {
		case 'artworks':
			return pixivMakeSaveInArcollectButton(pixiv_save_illust,save_id);
		case 'novel': {
			console.warn('TODO','Add support for novels page')
		} return;
		case 'series': {
			switch (canonical_link[canonical_link.length-3]) {
				default: {
					console.warn('TODO','Add support for',canonical_link[canonical_link.length-3],'series')
				} return;
			}
		} break;
		default: return; // Not an artwork
	}
}

// Trigger UI making
// TODO Watch for less mutations
new MutationObserver(pixivMakeSaveInArcollectUI).observe(document.querySelector('#root'),{
	'childList': true,
	'subtree': true,
});
pixivMakeSaveInArcollectUI();
