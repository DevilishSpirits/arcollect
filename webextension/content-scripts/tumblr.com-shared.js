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
/** \file tumblr.com-shared.js
 *  \brief Shared utils for Tumblr (https://tumbex.com/)
 *
 * Utilities common to Tumblr and their weird CDN proxies.
 */
 
/** Make a Tumblr's CDN friendly download specification
 * \param link to download
 * \return An Arcollect download specification that pass the proxy
 *
 * Tumblr's CDN is weird. It have the tendancy to return an HTML page instead of
 * the source image if not told do so with the [`Accept` header](https://developer.mozilla.org/docs/Web/HTTP/Headers/Accept).
 * It also detect foreign incoming traffic using the [`Referer`](https://developer.mozilla.org/docs/Web/HTTP/Headers/Referer)/).
 * Also [`If-Modified-Since`](https://developer.mozilla.org/docs/Web/HTTP/Headers/If-Modified-Since)/)/[`If-None-Match`](https://developer.mozilla.org/docs/Web/HTTP/Headers/If-None-Match)/)
 * cache checking headers are forbidden (why?!), like setting a .
 *
 * This function take care of that mess.
 */
function tumblr_make_image_download(link)
{
	return {
		'data': link,
		'headers': {
			// Force images, not an HTML preview
			'Accept': 'image/*',
			// Remove caching headers
			'If-Modified-Since': null,
			'If-None-Match': null,
			// Report traffic from the pseudo HTML page
			'Referer': link,
		},
	};
}
/** Return the best quality URL for a Tumblr image URL
 * \param link to the image URL
 * \return An Arcollect download specification to the highest quality version found
 *
 * There is two format of Tumblr CDN< URLs :
 * * **https://64.media.tumblr.com/<alnum>/tumblr_<alnum>_<width>.<ext>** that seem to be a legacy format
 * * **https://64.media.tumblr.com/<alnum>/<alnum-alnum>/s<width>x<height>/<alnum>.<ext>** that seem to be more recent
 *
 * If the format is not recognized, the returned download specification use the original link.
 *
 * \note Tumblr's CDN is weird. It have the tendancy to return an HTML page
 *       instead of the source image if not told do so with the [`Accept` header](https://developer.mozilla.org/docs/Web/HTTP/Headers/Accept).
 *       Also [`If-Modified-Since`](https://developer.mozilla.org/docs/Web/HTTP/Headers/If-Modified-Since)/)/[`If-None-Match`](https://developer.mozilla.org/docs/Web/HTTP/Headers/If-None-Match)/)
 *       cache checking headers are forbidden (why?!).
 *       This function take care of that.
 */
function tumblr_make_hq_download(link)
{
	// Alter the link
	if (/^https:\/\/.+media\.tumblr\.com\/[0-9a-z]+\/tumblr_[0-9a-z]+_[0-9]+\.[^.]+$/im.test(link)) {
		// Replace the width with 1280
		link = link.replace(/_[0-9]+\./,'_1280.');
	} else if (/^https:\/\/.+media.tumblr.com\/[a-z0-9]+\/[a-z0-9-]+\/s[0-9]+x[0-9]+\/[a-z0-9]+\.[^.]+$/im.test(link)) {
		// Request a reasonable large image
		link = link.replace(/\/s[0-9]+x[0-9]+\//i,'/s65535x65535/');
	} else {
		console.warning('Unrecognized Tumblr CDN link',link);
	}
	// Make the specification
	return tumblr_make_image_download(link);
}
