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

// Etablish connection with the native application on demand
var webext_adder_port_cache = null;
function webext_adder_port() {
	if (webext_adder_port_cache === null) {
		webext_adder_port_cache = browser.runtime.connectNative('arcollect_webext_adder');
		
		webext_adder_port_cache.onMessage.addListener(function(msg) {
			// Get the transaction id
			let transaction_id = msg['transaction_id'];
			// Relay msg
			transactions[transaction_id](msg);
			// Delete transaction
			delete transactions[transaction_id];
		});
		
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
		// Post message
		webext_adder_port().postMessage(msg);
	});
});
