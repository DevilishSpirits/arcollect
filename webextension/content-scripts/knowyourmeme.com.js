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


function KnowYourMeme_MakeWebextAdderPayload()
{
	/** Normalize source URL
	 *
	 * KnowYourMeme ignore the trailing '/' in the url. Remove it if present.
	 */
	let source = 'https://knowyourmeme.com'+window.location.pathname;
	if (source[source.length-1] == '/')
		source = source.slice(0,-1);
	
	/* Extract tags
	 *
	 * Tags are stored in a simple list.
	 */
	let tags = [...document.querySelectorAll('#tag_list a')].map(tag => {return{
		'id': tag.attributes['data-tag'].value.arcollect_tag(),
	}})
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
	let media_title = document.querySelector("#media-title");
	// Build the JSON
	let artworks = [{
		'title': media_title.textContent.trim().replaceAll('\n',' '),
		'source': source,
		// TODO 'rating': rating,
		'postdate': document.querySelector("#sidebar .row p > abbr.timeago[title]").title.replace('at ','').replace('PM',' PM'), // TODO Harden this
		'data': make_KnowYourMeme_img_cdn_downspec(document.querySelector("#photo_wrapper a").href),
	}];
	return {
		'platform': 'knowyourmeme.com',
		'artworks': artworks,
		'accounts': accounts,
		'tags': tags,
		// FIXME  'comics': comics, Should I consider the gallery as a comic ?
		'art_acc_links': Arcollect.simple_art_acc_links(artworks,{'account': accounts}),
		'art_tag_links': Arcollect.simple_art_tag_links(artworks,tags),
	};
}

/** Make the "Save in Arcollect" button
 */
function make_KnowYourMeme_save_ui() {
	let media_arrow_middle = document.querySelector("#media_arrows .middle");
	let saveButtonA = document.createElement("a");
	saveButtonA.className = "large red button gallery-button";
	new Arcollect.SaveControlHelper(saveButtonA,KnowYourMeme_MakeWebextAdderPayload,{
		'onSaveSuccess': async () => saveButtonA.className = "large green button gallery-button",
	});
	media_arrow_middle.appendChild(document.createTextNode(" ")); // We put a space to preserve the aesthetic
	media_arrow_middle.appendChild(saveButtonA);
}

make_KnowYourMeme_save_ui();
