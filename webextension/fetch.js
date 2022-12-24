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
/** \file webextension/fetch.js
 *  \brief API fetch()-ing wrappers.
 *
 * This file override [fetch() API](https://developer.mozilla.org/docs/Web/API/fetch)
 * to enhance the behavior of Arcollect regarding interaction with platform API.
 *
 * It perform the following modifications :
 *
 * * The `User-Agent` is prepended with `Arcollect/1.y (webextension)` to tell
 *   who we are.
 * * We use `content.fetch()` that behave like the `fetch()` in the website JS.
 *   This allow to betterly mimic the website API behavior.
 * * Enforce the use of HTTPS. It fails with an error if not.
 * * The `cache` is set to `force-cache` to avoid hitting the remote server even
 *   if the cache is stale. This is not a problem since the information found on
 *   the page are what is in the stale cache unless the site implement a complex
 *   partial update system (Twitter do this but we don't hit it anyway).
 *
 * \warning Don't perform useless API calls! Preserve remote servers please.
 *          Carefully follow directions of the website for using their APIs.
 *          Users are usually authenticated and may be banned if Arcollect
 *          misbehave!!! Mimic the behavior of the website own API calls.
 *
 * You may want to use fetch_json() that Promise a JSON for most REST API.
 */

// Override fetch
Arcollect.saved_fetch = content.fetch;
Arcollect.userAgent = 'Arcollect/'+browser.runtime.getManifest()['version']+' (webextension) '+navigator.userAgent;
function fetch(resource, init = {})
{
	// Sanitize URL
	resource = new URL(resource);
	if (resource.protocol !== 'https:') {
		console.error('Attempted to fetch() non https:// API:',resource);
		return Promise.reject(browser.i18n.getMessage('webext_non_https_fetch').replace('%URL%',resource.href));
	}
	// Configure headers
	if (!init.hasOwnProperty('headers'))
		init.headers = {};
	init.headers['User-Agent'] = Arcollect.userAgent;
	init.cache = 'force-cache';
	return Arcollect.saved_fetch(resource,init);
}
function fetch_json(resource, init = {})
{
	if (!init.hasOwnProperty('headers'))
		init.headers = {};
	init.headers['Accept'] = 'application/json';
	// We don't use response.text() and then parse to avoid X-Ray troubles. Since
	// This object is from the page, not the webextension.
	return fetch(resource,init).then(response => response.text()).then(JSON.parse);
}
