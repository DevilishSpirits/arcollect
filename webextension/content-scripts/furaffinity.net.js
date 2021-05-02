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
	let submissionImg = document.getElementById('submissionImg');
	let avatarImg = document.getElementsByClassName('submission-user-icon floatleft avatar')[0];
	let authorName = avatarImg.parentElement.href.split('/')[4]; // Extract from the user avatar URL
	let description = document.getElementsByClassName('submission-description user-submitted-links')[0].innerText;
	// Asynchronously save
	arcollect_download_image(submissionImg.src).then(function(image_data) {
		// Create a JSON
		return {
			'platform': 'furaffinity.net',
			'artworks': [{
				'title': submissionImg.alt,
				'desc': description,
				'source': window.location.origin+window.location.pathname,
				'data': image_data
			}]/*,
			'author': [{
				'id': authorName,
				'name': authorName,
				'avatar': avatarImg.src,
			}]*/
		};
	}).then(function(data) {
		return arcollect_submit(data);
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
