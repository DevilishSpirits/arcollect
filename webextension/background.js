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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
/** \file webextension/background.js
 *  \brief Webextension background main script
 */

/** Look for the DNS API and use an empty stub
 */
function dns_resolve_stub(hostname,flags) {
	return Promise.reject('browser.dns API unavailable')
}
if (!browser.dns)
	browser = {dns: dns_resolve_stub};
else if (!browser.dns.resolve)
	browser.dns.resolve = dns_resolve_stub;

/** List of running transactions
 *
 * This list track of transactions to the native webext-adder generated by
 * arcollect_submit() in content-scripts.
 *
 * These are sendReponse arguments from the content-script onMessage callback.
 */
var transactions = {};
/** Next transactions id
 *
 * This integer is used to generate unique transactions id to distinguish
 * between differents transactions. It is used to generate keys for 
 * #transactions.
 */
var next_transaction_id = 0;

/** Answer a content-script request
 * \param answer The JSON to answer
 *
 * The routing is assured by `transaction_id` inside.
 */
function reply_to_content_script(answer) {
	let transaction_id = answer.transaction_id;
	if (transactions.hasOwnProperty(transaction_id)) {
		transactions[transaction_id](answer);
		delete transactions[transaction_id];
	} else console.error('reply_to_content_script(): transaction_id is unknown.',answer);
}

/** Cached native-messaging port
 *
 * Cached result of webext_adder_port() for his internal use only. 
 */
var webext_adder_port_cache = null;
/** Get the webext-adder native-messaging port and connect on demand
 * \return The [`runtime.Port`](https://developer.mozilla.org/docs/Mozilla/Add-ons/WebExtensions/API/runtime/Port) to the webext-adder
 */
function webext_adder_port() {
	if (webext_adder_port_cache === null) {
		webext_adder_port_cache = browser.runtime.connectNative('arcollect_webext_adder');
		
		webext_adder_port_cache.onMessage.addListener(reply_to_content_script);
		
		webext_adder_port_cache.onDisconnect.addListener(function(port) {
			// Reset webext_adder_port_cache
			webext_adder_port_cache = null;
			// Make error message
			let errmsg = 'Connection with the Arcollect artwork adder has been lost.';
			if (port.error)
				errmsg += ` ${port.error.message}.`;
			// Log
			console.error(errmsg);
			// Notify transactions of connection lose and end them
			for (key of Object.keys(transactions)) 
				transactions[key]({
					'success': false,
					'transaction_id': key,
					'reason': errmsg
				});
			// Clear list of transactions
			transactions = {};
		});
	}
	return webext_adder_port_cache;
}

/** Get the URL hit by the webext adder for a gven download specification
 * \param spec to read from
 * \return The URL() object that will be hit by the webext adder, or undefined
 *         if the spec is undefined.
 */
function get_download_spec_url(spec) {
	switch (typeof(spec)) {
		case 'object': {
			spec = spec.data;
		} break;
		case 'undefined':return undefined;
	}
	return new URL(spec);
}
function extract_hostname(spec,domains) {
	spec = get_download_spec_url(spec);
	if (spec != undefined)
		domains.add(spec.hostname);
}

// Setup content-scripts calls
browser.runtime.onConnect.addListener(function(port) {
	console.log(`Content-script connection from ${port.sender.url}`);
	/*
	port.onDisconnect.addListener(function() {
		delete transactions[TODO Find the good id to remove];
	});
	*/
	
	port.onMessage.addListener(function(msg) {
		// Set the transaction id
		let transaction_id = msg['transaction_id'] + ' ' +next_transaction_id++;
		msg['transaction_id'] = transaction_id
		transactions[transaction_id] = port.postMessage;
		
		new Promise(function(resolve,reject) {
			// Process trough platform specific post-processor
			if (msg.platform == 'twitter.com') {
				// TODO Convert it to Promises
				msg = twitter_post_process_submit(msg);
				if (msg != null)
					resolve(msg);
				else reject('Twitter specific processing failed');
			} else resolve(msg);
		}).then(function(msg) {
			// List domains there
			let domains = new Set();
			if (msg.hasOwnProperty('artworks'))
				msg.artworks.forEach(function(artwork) {
					extract_hostname(artwork.data,domains);
					extract_hostname(artwork.thumbnail,domains);
				});
			if (msg.hasOwnProperty('accounts'))
				msg.accounts.forEach(function(account) {
					extract_hostname(account.icon,domains);
				});
			// Generate DNS promises
			msg.dns_prefill = [];
			let res_promises = [Promise.resolve(msg)]; 
			for (const domain of domains) {
				msg.dns_prefill.push(domain);
				res_promises.push(browser.dns.resolve(domain));
			}
			return Promise.allSettled(res_promises);
		}).then(function(res) {
			[msg, ...promises] = res;
			msg = msg.value;
			let domains = msg.dns_prefill;
			msg.dns_prefill = {};
			domains.forEach(function(domain,index) {
				let promise = promises[index];
				if ((promise.status == 'fulfilled')&&(promise.value.addresses.length > 0)) {
					msg.dns_prefill[domain] = promise.value.addresses
				}
			});
			return msg;
		// Final filter before sending
		}).then(function(msg){
			// Post message
			console.log(msg);
			return webext_adder_port().postMessage(msg);
		}).catch(function(reason) {
			return reply_to_content_script({
				'success': false,
				'transaction_id': transaction_id,
				'reason': reason
			});
		});
	});
});
