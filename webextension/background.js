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
// Platform dictionary of cached result for opportunistic information gathering
cached_results = {};
// Port array by platform dict
ports_by_platform = {};

urlmatch_platform = {
	"https:\/\/www.furaffinity.net\/view\/.*\/": "furaffinity.net"
};
function lookup_platform_from_url(url)
{
	for ([key, value] of Object.entries(urlmatch_platform)) {
		if (url.match(key))
			return value;
	};
	console.log("lookup_platform_from_url: Unknow platform \""+url+"\"");
	return null;
}

// Etablish connection with the native application on demand
var webext_adder_port_cache = null;
function webext_adder_port() {
	if (webext_adder_port_cache === null) {
		webext_adder_port_cache = browser.runtime.connectNative('arcollect_webext_adder');
	}
	return webext_adder_port_cache;
}

// Setup content-scripts calls
browser.runtime.onConnect.addListener(function(port) {
	port.arcollect_platform = lookup_platform_from_url(port.sender.url);
	console.log(`Content-script connection from ${port.sender.url} (${port.arcollect_platform})`);
	port.onMessage.addListener(function(msg) {
		webext_adder_port().postMessage(msg);
	});
});
