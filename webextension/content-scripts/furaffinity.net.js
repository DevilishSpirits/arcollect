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
/* FIXME Rewrite with extensive commenting */
/* Arcollect content script for FurAffinity
 */
/** Save the artwork
 */
var save_buttondiv = null;
function do_save_artwork()
{
	// Show that we are saving the artwork
	// TODO Set opacity to halfway
	save_buttondiv.onclick = null;
	save_buttondiv.text = 'Saving...';
	// Parse the page
	// TODO Find "indesc" relations
	let submissionImg = document.getElementById('submissionImg');
	let avatarImg = document.getElementsByClassName('submission-user-icon floatleft avatar')[0];
	let accountName = avatarImg.parentElement.href.split('/')[4]; // Extract from the user avatar URL
	let description = document.getElementsByClassName('submission-description user-submitted-links')[0].innerText;
	let ratingBoxClass = document.getElementsByClassName('rating-box')[0].className;
	let accountJSON = [{
		'id': accountName,
		'name': accountName,
		'url': avatarImg.parentElement.href,
		'icon': avatarImg.src
	}];
	let art_acc_links = [{
		'account': accountName,
		'artwork': window.location.origin+window.location.pathname,
		'link': 'account'
	}];
	let tags = []
	let art_tag_links = []
	tags_rows = document.getElementsByClassName('tags-row')[0].getElementsByTagName('a');
	for (let i = 0; i < tags_rows.length; i++) {
		tags.push({
			'id': tags_rows[i].text
		});
		art_tag_links.push({
			'artwork': window.location.origin+window.location.pathname,
			'tag': tags_rows[i].text
		});
	}
	arcollect_submit({
			'platform': 'furaffinity.net',
			'artworks': [{
				'title': submissionImg.alt,
				'desc': description,
				'source': window.location.origin+window.location.pathname,
				'rating': ratingBoxClass.includes('adult') ? 18 : ratingBoxClass.includes('mature') ? 16 : 0,
				'data': submissionImg.src
			}],
			'accounts': accountJSON,
			'tags': tags,
			'art_acc_links': art_acc_links,
			'art_tag_links': art_tag_links,
	}).then(function() {
		save_buttondiv.text = 'Saved';
	}).catch(function(reason) {
		save_buttondiv.onclick = do_save_artwork;
		save_buttondiv.text = 'Retry to save in Arcollect';
		console.log('Failed to save in Arcollect ! '+reason);
		alert('Failed to save in Arcollect ! '+reason);
	});
}

function make_save_ui() {
	let button_nav = document.getElementsByClassName('aligncenter auto_link hideonfull1 favorite-nav');
	if (button_nav.length == 1) {
		button_nav = button_nav[0];
		// Create our button
		save_buttondiv = document.createElement("a");
		save_buttondiv.text = "Save in Arcollect";
		save_buttondiv.className = "button standard mobile-fix";
		save_buttondiv.onclick = do_save_artwork;
		// Append our button in the <div>
		// TODO Try to add it next to the download button
		button_nav.append(save_buttondiv);
	} else console.log('Arcollect error ! Found '+button_nav.length+' element(s) with class "aligncenter auto_link hideonfull1 favorite-nav".');
}

make_save_ui();
