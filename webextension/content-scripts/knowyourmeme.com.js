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
/** \file webextension/content-scripts/knowyourmeme.com.js
 *  \brief Content script for KnowYourMeme (https://knowyourmeme.com/)
 *
 * This platform have a rather clean HTML.
 *
 * I added support for this thing because it's fun.
 *
 * \todo Shortcut like and Protip like on e621 overkill integration :3.
 */

/** Save button <a> element
 *
 * It is changed by save_KnowYourMeme_meme() to reflect saving progression
 */
var saveButtonA = null;

/** Make a download spec for an image on the KnowYourMeme's CDN
 *
 * This function ask for the original image and return the URL as a string.
 * The URL format is `https://i.kym-cdn.com/<section>/<section>/<size>/<3 digits>/<3 digits>/<3 digits>/<3 digits>.<ext>`
 * and we set size to `original`.
 */
function make_KnowYourMeme_img_cdn_downspec(url) {
	url = url.split('/');
	url[5] = 'original';
	return url.join('/');
}

/** Save the artwork
 */
function save_KnowYourMeme_meme()
{
	// Show that we are saving the artwork
	saveButtonA.onclick = null;
	saveButtonA.text = arco_i18n_saving;
	saveButtonA.className = "large red button gallery-button";
	saveButtonA.style = 'cursor:progress;';
	
	/** Normalize source URL
	 *
	 * KnowYourMeme ignore the trailing '/' in the url. Remove it if present.
	 */
	let source = 'https://knowyourmeme.com/'+window.location.pathname;
	if (source[source.length-1] == '/')
		source = source.slice(0,-1);
	
	/* Extract tags
	 *
	 * Tags are stored in a simple list.
	 */
	let tags = []
	let art_tag_links = []
	for (let tag of document.querySelectorAll('#tag_list a')) {
		let tag_id = tag.attributes['data-tag'].value.arcollect_tag();
		tags.push({
			'id': tag_id,
		})
		art_tag_links.push({
			'artwork': source,
			'tag': tag_id,
		})
	}
	/* Extract account
	 *
	 * We have all the infos of the uploader.
	 */
	let author_section = document.querySelector('#author_info section');
	let account_id = author_section.getElementsByTagName('a')[0].href.split('/')[4];
	let accounts = [{
		"id": account_id,
		"name": author_section.querySelector('h6 a').text,
		"url": "https://knowyourmeme.com/users/"+account_id,
		"desc": author_section.querySelector('p.role').textContent.trim(),
		"icon": make_KnowYourMeme_img_cdn_downspec(author_section.querySelector('img').src),
	}];
	let art_acc_links = [{
		"artwork": source,
		"account": account_id,
		"link": "account"
	}];
	let media_title = document.querySelector("#media-title");
	// Build the JSON
	submit_json = {
		'platform': 'knowyourmeme.com',
		'artworks': [{
			'title': media_title.textContent.trim().replaceAll('\n',' '),
			'source': source,
			// TODO 'rating': rating,
			'postdate': Date.parse(document.querySelector("#sidebar .row p > abbr.timeago[title]").title.replace('at ','').replace('PM',' PM'))/1000, // TODO Harden this
			'data': make_KnowYourMeme_img_cdn_downspec(document.querySelector("#photo_wrapper a").href),
		}],
		'accounts': accounts,
		'tags': tags,
		// FIXME  'comics': comics, Should I consider the gallery as a comic ?
		'art_acc_links': art_acc_links,
		'art_tag_links': art_tag_links,
	};
	// Submit
	arcollect_submit(submit_json).then(function() {
		saveButtonA.text = arco_i18n_saved;
		saveButtonA.className = 'large green button gallery-button';
		saveButtonA.style = 'cursor:default;';
	}).catch(function(reason) {
		saveButtonA.onclick = save_KnowYourMeme_meme;
		saveButtonA.text = arco_i18n_save_retry;
		saveButtonA.className = 'large red button gallery-button';
		saveButtonA.style = 'cursor:pointer;';
		console.error(arco_i18n_save_fail+' '+reason);
		alert(arco_i18n_save_fail+' '+reason);
	});
}

/** Make the "Save in Arcollect" button
 */
function make_KnowYourMeme_save_ui() {
	let media_arrow_middle = document.querySelector("#media_arrows .middle");
	saveButtonA = document.createElement("a");
	saveButtonA.text = arco_i18n_save;
	saveButtonA.className = "large red button gallery-button";
	saveButtonA.style = 'cursor:pointer;';
	saveButtonA.onclick = save_KnowYourMeme_meme;
	media_arrow_middle.appendChild(document.createTextNode(" ")); // We put a space to preserve the aesthetic
	media_arrow_middle.appendChild(saveButtonA);
}

make_KnowYourMeme_save_ui();
