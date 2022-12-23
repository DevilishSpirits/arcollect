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
/** \file common.js
 *  \brief Common header for content-scripts
 *
 * This file is effectively a JavaScript "header" that declare some useful
 * functions like arcollect_submit().
 */

/** Arcollect internal API
 */
class Arcollect {
	/** Background script connection
	 *
	 * At startup, the script automatically connect to the background script
	 */
	static port = null;
	/** List of running transactions
	 *
	 * This list track of transactions to the native webext-adder made by
	 * arcollect_submit().
	 */
	static transactions = {};
	/** Next transactions
	 *
	 * This integer is used to generate unique transactions id to distinguish
	 * between differents transactions. It is used to generate keys for 
	 * #Arcollect.transactions.
	 */
	static next_transaction_id = 0;
	/** Normalize tag id
	 * \param tag The image URL
	 * \return The tag in a normalized form
	 *
	 * Perform tag normalization, replacing everything that is not a letter or a
	 * digit or a question/exclamation marks with '-' and force lowercase.
	 */
	static normalize_tag(tag){
		return tag.match(/(\p{L}|\p{N}|\?|!)+/gu).join('-').toLowerCase();
	};
	/** Wordlist of NSFW tags
	 * 
	 * This is a list of tags that should be considered as flag for adult content.
	 *
	 * It is used on platforms that lack efficient adult rating system like Tumblr
	 * or Twitter.
	 *
	 * You might expand this list, but the root of the problem is located in the
	 * platforms themselves.
	 */
	static adult_tags = [
		'nsfw',
		'porn',
		'sex',
	];
	/** File extension to MIME type dictionary
	 *
	 * Only contain supported formats.
	 */
	static mime_by_ext = {
		'gif' : 'image/gif',
		'jpe' : 'image/jpeg',
		'jpg' : 'image/jpeg',
		'jpeg': 'image/jpeg',
		'png' : 'image/png',
		'rtf' : 'text/rtf',
	};
	/** Submit new data into the database
	 *
	 * \param json_object Objects to send.
	 * \return A promise
	 * \see Read webext-adder/README.md for the data format.
	 */
	static submit(json_object) {
		return new Promise(
			function(resolve, reject) {
				// Generate a transaction id
				let transaction_id = (Arcollect.next_transaction_id++).toString();
				json_object['transaction_id'] = transaction_id;
				Arcollect.transactions[transaction_id] = {
					'resolve': resolve,
					'reject': reject,
				};
				// Send message
				console.log('Arcollect.submit',json_object)
				Arcollect.port.postMessage(json_object);
			}
		);
	};
	
	/** Generate a xxx_acc_links from the list and accounts list
	 * \param xxx array in it's final form
	 * \param xxx_idname is the name of the *id* field (`"source"` on artworks)
	 * \param xxx_linkid is the name of the link item id (`"artwork"` on artworks)
	 * \param accounts dictionary that map link type to an array of accounts
	 * \return The `xxx_acc_links` array
	 *
	 * Template function for most case of simple_xxx_acc_links functions.
	 * \see Arcollect.simple_art_acc_links that wrap this function for a practical
	 *      example.
	 */
	static simple_xxx_acc_links(xxx,xxx_idname,xxx_linkid,accounts) {
		// TODO Use a flat()/flatMap() when requiring Firefox 62
		return [].concat(...([].concat(...(Object.entries(accounts).map(acc_map => acc_map[1].map(account => {return Object.entries({
				"account": account.id,
				"link": acc_map[0],
			});
		})))).map(link_template => xxx.map(item => Object.fromEntries([[xxx_linkid, item[xxx_idname]],...link_template])))))
	};
	/** Generate the art_acc_links from artworks and accounts list
	 * \param artworks array in it's final form
	 * \param accounts dictionary that map link type to an array of accounts
	 * \return The `art_acc_links` array
	 *
	 * Convenience function for most case of art_acc_links when it's just linking
	 * accounts to all artworks.
	 *
	 * Here an example usage below with `artwork1`, `artwork2` and a
	 *          `postingAccount` plus an array of `mentions`.
	 * \code{.js}
	 * art_acc_links = Arcollect.simple_art_acc_links([artwork1,artwork2],{
	 * 	"account": [postingAccount]
	 * 	"indesc": mentions,
	 * })
	 * \endcode
	 */
	static simple_art_acc_links = (artworks,accounts) => this.simple_xxx_acc_links(artworks,'source','artwork',accounts);
	/** Generate the com_acc_links from artworks and comics list
	 * \param comics array in it's final form
	 * \param accounts dictionary that map link type to an array of accounts
	 * \return The `com_acc_links` array
	 *
	 * Convenience function for most case of com_acc_links when it's just linking
	 * accounts to all comics.
	 *
	 * \see Arcollect.simple_art_acc_links that is the same function for linking
	 * artworks to accounts.
	 */
	static simple_com_acc_links = (comics,accounts) => this.simple_xxx_acc_links(comics,'id','comic',accounts);
};

// TODO Wrap this part
const arco_i18n_save       = browser.i18n.getMessage('webext_save_in_arcollect');
const arco_i18n_saving     = browser.i18n.getMessage('webext_saving_in_arcollect');
const arco_i18n_save_fail  = browser.i18n.getMessage('webext_save_in_arcollect_failed');
const arco_i18n_save_retry = browser.i18n.getMessage('webext_save_in_arcollect_retry');;
const arco_i18n_saved      = browser.i18n.getMessage('webext_saved_in_arcollect');;

// Alter string prototype
String.prototype.arcollect_tag = function() {
	return Arcollect.normalize_tag(this);
}
String.prototype.is_adult_tag = function() {
	return Arcollect.adult_tags.includes(this.toLowerCase());
}

// Open the port
Arcollect.port = browser.runtime.connect();

// Answer the associated Promise
Arcollect.port.onMessage.addListener(function(msg) {
	// Get the transaction id
	// Note: The background add it's own transaction_id after our one, strip it
	let transaction_id = msg['transaction_id'].split(' ')[0];
	if (msg['success'])
		Arcollect.transactions[transaction_id].resolve();
	else Arcollect.transactions[transaction_id].reject(msg['reason']);
	delete Arcollect.transactions[transaction_id];
});
