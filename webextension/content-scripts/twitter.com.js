/* Arcollect -- An artwork collection manager
 * Copyright (C) 20XX <You-name>
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
/** \file webextension/content-scripts/twitter.com.js
 *  \brief Content script for Twitter (https://twitter.com/)
 *
 * Twitter HTML is extremely messy. We use accessibility features in-order to
 * locate where to insert the Save in Arcollect button and listen ot the API
 * in order to extract 
 *       call in a background script which have a special support for Twitter;
 * \see The webextension/twitter.js background script that listen to API calls.
 */

/** Twitter layer element (`<div id="layers">`)
 *
 * Twitter use this as a root for modal windows
 */
var layersDiv = document.getElementById('layers');

/** Save the artwork
 */
function save_twitter_artwork(button, text_button, submit_json, retry_to_save)
{
	text_button.innerHTML = 'Saving...';
	arcollect_submit(submit_json).then(function() {
		text_button.innerHTML = 'Saved';
	}).catch(function(reason) {
		button.onclick = retry_to_save;
		text_button.innerHTML = 'Retry to save in Arcollect';
		console.error('Failed to save in Arcollect ! '+reason);
		alert('Failed to save in Arcollect ! '+reason);
	});
}
/** Save the artwork
 */
function save_twitter_artwork_by_urls(button, text_button, urls, retry_to_save)
{
	let artworks = []
	for (i in urls)
		artworks.push({'source':urls[i]});
	// Build the JSON
	save_twitter_artwork(button,text_button,{
		'platform': 'twitter.com',
		'artworks': artworks,
	},retry_to_save);
}

function make_twitter_photo_save_button_clicked(e) {
	let source = window.location.origin+window.location.pathname;
	save_twitter_artwork_by_urls(this,this.querySelector("[data-testid=app-text-transition-container]"),[source],make_twitter_photo_save_button_clicked);
	e.stopPropagation();
}
/** Make the "Save in Arcollect" button in photo full-view
 */
function make_twitter_photo_save_button() {
	/** Twitter modal header (`<div aria-labelledby="modal-header">`)
	 *
	 * Twitter use this as a root for the modal window we want.
	 */
	let modal_header = layersDiv.querySelector('div[aria-labelledby="modal-header"]');
	/** Twitter image view main pane
	 *
	 * Thee main pane is the part in full-screen excluding the hideable right-pane.
	 * We focus on this pane because further matching would match other tweets HTML.
	 */
	let main_pane = modal_header.childNodes[0].childNodes[0];
	/** Twitter action bar
	 *
	 * The bar that allows you to reply, retweet, like and share.
	 */
	let buttons_bar = main_pane.querySelector('div[role=group]:not([aria-roledescription="carousel"])');
	/** Save in Arcollect button
	 */
	let new_button = buttons_bar.childNodes[0].cloneNode(deep = true);
	// Remove the icon if any
	let button_svg = new_button.querySelector('svg')
	if (button_svg != null)
		button_svg.parentNode.remove()
	// Set text
	new_button.querySelector("[data-testid=app-text-transition-container]").innerHTML = "Save in Arcollect";
	// Append the button
	buttons_bar.appendChild(new_button);
	new_button.onclick = make_twitter_photo_save_button_clicked;
}

/** Called when we found `<div id="layers">`
 *
 * This function perform some actions which require a valid #layersDiv and is
 * called when it is found.
 */
function gotLayersDiv() {
	// Observe the child list
	new MutationObserver(make_twitter_photo_save_button).observe(layersDiv,{'childList': true});
}

if (layersDiv == null) {
	// `<div id="layers">` is not created, poll for it's creation
	document.firstElementChild.onmousemove = function() {
		layersDiv = document.getElementById('layers');
		if (layersDiv != null) {
			// Found `<div id="layers">`!
			gotLayersDiv();
			document.firstElementChild.onmousemove = null;
		}
	}
} else gotLayersDiv();
