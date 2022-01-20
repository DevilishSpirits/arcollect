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
/** \file tumbex.com.js
 *  \brief Content script for Tumbex (https://tumbex.com/)
 *
 * This platform have a clean HTML and even put in comments the result of
 * Tumblr's API call, such a lovely platform.
 * \todo Allow to save parts of a post.
 * \todo Save mentions.
 */
var tumbex_accounts_cache = {};
function tumbex_cache_account_article(article)
{
	let data = JSON.parse(article.childNodes[0].textContent);
	tumbex_accounts_cache[data['name']] = {
		'id'   : data['name'],
		'name' : data['name'],
		'title': data['title'],
		'icon' : tumblr_make_image_download(data['avatar']),
		'url'  : 'https://www.tumbex.com/'+data['name']+'.tumblr/posts',
		'desc' : article.getElementsByClassName('blog-brick-body')[0].contentText, // The API returns HTML, don't parse it and read the DOM instead.
		'tumblr.nsfw': data['nsfw'],
	};
}
function tumbex_lookup_account_articles()
{
	for (let article of document.querySelectorAll('article.blog-brick'))
		tumbex_cache_account_article(article);
}

function tumbex_account(id) {
	
	if (!(id in tumbex_accounts_cache))
		tumbex_lookup_account_articles();
	return tumbex_accounts_cache[id];
}

/** Save the post
 */
function tumbex_save_article(article)
{
	// Parse the JSON in comment
	let data = JSON.parse(article.childNodes[0].textContent);
	// Show that we are saving the artwork
	if (!article.arcollect_save_label) {
		// Create the underlaying <small> label
		article.arcollect_save_label = document.createElement('small');
		// Create the <a class="btn">
		let post_actions = article.getElementsByClassName('post-actions')[0];
		let btn = document.createElement('a');
		btn.className = 'btn';
		btn.insertBefore(article.arcollect_save_label,null);
		// Insert in the page
		post_actions.insertBefore(btn,post_actions.firstChild);
	}
	article.arcollect_save_label.textContent = arco_i18n_saving;
	article.arcollect_save_label.style = 'cursor:progress;';
	
	/** Generate source URL
	 */
	if (data['slug'] == null)
		// Replace a null slug by an empty string
		data['slug'] = '';
	let source = 'https://www.tumbex.com/'+data.tumblr+'.tumblr/post/'+data['$id']+'/'+data['slug'];
	
	/** Extract tags
	 *
	 * It's in "tags"... seriously that's super easy.
	 */
	data.tags = data.tags.map(arcollect_normalize_tag); // Normalize tags
	let tags = []
	let art_tag_links = []
	let rating = 0;
	for (let tag of data.tags) {
		if (tag.is_adult_tag())
			// Tumblr have no post rating system, rely on the adult tags list.
			is_nsfw = 18;
		tags.push({
			'id': tag,
		});
	}
	
	/** Extract account informations
	 */
	let account = tumbex_account(data.tumblr);
	if (account['tumblr.nsfw'])
		rating = 18;
	
	/** Extract contents
	 *
	 * Tumblr store pictures and descriptions in a big "blocks" with "content"
	 * inside, we iterate over and save everything.
	 */
	let images = [];
	let texts = [];
	for (let block of data['blocks']) {
		for (let content of block['content'])
			switch (content['type']) {
				case 'image': {
					images.push(content['hd']);
				} break;
				case 'text': {
					texts.push(content['text']);
				} break;
			}
	}
	
	/** Process extracted contents
	 */
	let title = null;
	let description = '';
	if (texts.length > 0)
		description = texts.shift().split('\n')
		title = description.shift();
		description = description.join('\n')
	description += texts.join('\n');
	let artworks = [];
	let comic_pages = {};
	let artworkno = 0;
	for (let artwork of images) {
		artworkno += 1;
		let artsource = source+'#'+artworkno;
		artworks.push({
			'title': title,
			'desc': description,
			'source': artsource,
			'rating': rating,
			'postdate': data['$timestamp'],
			'data': tumblr_make_hq_download(artwork)
		})
		// Populare art_tag_links
		for (let tag of data.tags)
			art_tag_links.push({
				'artwork': artsource,
				'tag': tag
			});
		// Populate comic page
		comic_pages[artsource] = {'page': artworkno};
	}
	if (artworkno == 1) {
		// There is only one artwork, don't add a #artX in the source.
		artworks[0]['source'] = source;
		art_tag_links.length = 0;
		for (let tag of data.tags)
			art_tag_links.push({
				'artwork': source,
				'tag': tag
			});
		comic_pages = null;
	}
	
	// Build the JSON
	submit_json = {
		'platform': 'tumbex.com',
		'artworks': artworks,
		'accounts': [account],
		'tags': tags,
		'comics': (comic_pages != null) ? [{'pages': comic_pages,}] : [],
		'art_acc_links': artworks.map(function(artwork){return {'artwork': artwork['source'], 'account': data.tumblr, 'link': 'account'};}),
		'art_tag_links': art_tag_links,
	};
	
	// Submit
	arcollect_submit(submit_json).then(function() {
		article.arcollect_save_label.textContent = arco_i18n_saved;
		article.arcollect_save_label.style = 'cursor:default;';
	}).catch(function(reason) {
		article.arcollect_save_label.textContent = arco_i18n_save_fail;
		console.error(arco_i18n_save_fail+' '+reason);
		alert(arco_i18n_save_fail+' '+reason);
	});
}
function tumbex_save_button_pressed() {
	return tumbex_save_article(this.post);
}

/** Insert the "Save in Arcollect" button
 * \param post The `<article class="post">` to edit
 */
function tumbex_make_save_ui(post) {
	if (!post.saveButtonA) {
		let dropdown = post.getElementsByClassName("dropdown-menu-right")[0];
		post.saveButtonA = document.createElement('a');
		post.saveButtonA.className = "post-action dropdown-item";
		post.saveButtonA.text = arco_i18n_save;
		post.saveButtonA.style = 'cursor:pointer;';
		post.saveButtonA.onclick = tumbex_save_button_pressed;
		post.saveButtonA.post = post;
		
		dropdown.insertBefore(post.saveButtonA,dropdown.firstChild);
	}
}

function tumbex_make_save_uis() {
	for (let post of document.querySelectorAll('article.post'))
		tumbex_make_save_ui(post);
}

let posts_observer = new MutationObserver(tumbex_make_save_uis);
posts_observer.observe(document.children[0],{'childList': true, 'subtree': true});
tumbex_make_save_uis();
