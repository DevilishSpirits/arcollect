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
		'icon': arcollect_download_image(avatarImg.src)
	}];
	let art_acc_links = [{
		'account': accountName,
		'artwork': window.location.origin+window.location.pathname,
		'link': 'account'
	}];
		// Download account images
	Promise.all(accountJSON.map(function(element) {
		return element['icon'];
	})).then(function(results){
		// Set accounts icons
		accountJSON.forEach(function(element,index) {
			element['icon'] = results[index];
		});
		// Download the main artwork
		return arcollect_download_image(submissionImg.src);
	}).then(function(image_data) {
		return arcollect_submit({
			'platform': 'furaffinity.net',
			'artworks': [{
				'title': submissionImg.alt,
				'desc': description,
				'source': window.location.origin+window.location.pathname,
				'rating': ratingBoxClass.includes('adult') ? 18 : ratingBoxClass.includes('mature') ? 16 : 0,
				'data': image_data
			}],
			'accounts': accountJSON,
			'art_acc_links': art_acc_links,
		});
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

arcollect_has_artwork(window.location.origin+window.location.pathname).then(function(result) {
	if (result) {
		// Do nothing
	} else {
		make_save_ui();
	}
})
