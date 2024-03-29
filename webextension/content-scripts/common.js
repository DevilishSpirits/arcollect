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
	/** Convert hexadecimal data to Base64
	 * \param data in hexadecimal encoding
	 * \return data in Base64 encoding
	 *
	 * This is a utility function mostly useful for SRI.
	 */
	static h2a(data) {
		return btoa(String.fromCharCode(...Uint8Array.from(data.matchAll('..'),hex => parseInt(hex,16))));
	}
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
	/** Detect the page Referrer-Policy
	 * \return The Referrer-Policy for the webext-adder or null if none is found
	 * \todo We can't get the Referrer-Policy defined in HTTP headers
	 */
	static detectReferrerPolicy() {
		// Look for <meta name="referrer"/>
		let meta_node = document.querySelector('meta[name="referrer"]');
		if (meta_node && meta_node.content)
			return meta_node.content;
		// Nothing found, bail out a null;
		return null;
	}
	/** Submit new data into the database
	 *
	 * \param json_object Objects to send.
	 * \return A promise
	 * \see Read webext-adder/README.md for the data format.
	 */
	static submit(json_object) {
		return new Promise(
			function(resolve, reject) {
				// Set the Referrer-Policy
				if (!json_object.hasOwnProperty('referrer_policy')) {
					// Look for <meta name="referrer"/>
					let referrer_policy = Arcollect.detectReferrerPolicy();
					if (referrer_policy)
						json_object['referrer_policy'] = referrer_policy;
				}
				// Generate a transaction id
				let transaction_id = (Arcollect.next_transaction_id++).toString();
				json_object['transaction_id'] = transaction_id;
				Arcollect.transactions[transaction_id] = {
					'resolve': resolve,
					'reject': reject,
				};
				// Perform convenience post-processings
				if (json_object.artworks)
					json_object.artworks.forEach(function(artwork) {
						if (artwork.data)
							artwork.data = Arcollect.makeDownloadSpec(artwork.data);
						if (artwork.thumbnail)
							artwork.thumbnail = Arcollect.makeDownloadSpec(artwork.thumbnail);
						if (artwork.postdate)
							artwork.postdate = Arcollect.parseTimestamp(artwork.postdate);
					});
				if (json_object.accounts)
					json_object.accounts.forEach(function(account) {
						if (account.icon)
							account.icon = Arcollect.makeDownloadSpec(account.icon);
						if (account.createdate)
							account.createdate = Arcollect.parseTimestamp(account.createdate);
					});
				if (json_object.tags)
					json_object.tags.forEach(function(tag) {
						if (tag.createdate)
							tag.createdate = Arcollect.parseTimestamp(tag.createdate);
					});
				if (json_object.comics)
					json_object.comics.forEach(function(comic) {
						if (comic.postdate)
							comic.postdate = Arcollect.parseTimestamp(comic.postdate);
					});
				// Send message
				console.log('Arcollect.submit',json_object)
				Arcollect.port.postMessage(json_object);
			}
		);
	};
	
	/** Generate a download specification from something
	 * \param input thing to make a download spec from
	 * \param settings optional dictionary that will be merged with the result
	 * \return A download specification for the input.
	 *
	 * This function is a convenience function to make a download specification
	 * from various objects, generating a full fledged specification that will
	 * closely mimic the browser behavior when possible. Possible inputs are:
	 * - *Strings* that might returned as is or in a object with settings applied.
	 * - *JSON object* that are returned with settings applied
	 * - *URL objects* that are converted to strings
	 * - *<a> elements*
	 * - *<img> elements* where the src attribute is extracted
	 *
	 * This function understand the "referrerPolicy" and "rel" attributes.
	 */
	static makeDownloadSpec(input,settings) {
		
		// Check if there's a specific Referrer-Policy
		if (input.referrerPolicy && input.referrerPolicy != '')
			settings = Object.assign({"referrer_policy": input.referrerPolicy},settings);
		if (input.rel && input.rel.split(' ').includes('noreferrer'))
			settings = Object.assign({"referrer_policy": 'no-referrer'},settings);
		
		// Extract the link from the input
		if (input instanceof URL)
			// new URL - Extract the "href" property
			input = input.href;
		else if (input instanceof HTMLAnchorElement)
			// <a> - Extract the "href" attribute
			input = input.href;
		else if (input instanceof HTMLImageElement)
			// <img> - Extract the "src" attribute
			input = input.src;
		else if (input instanceof Element)
			// Throw when we don't support an Element
			throw "<"+input.tagName.toLowerCase()+"> elements are not supported.";
		else if (input instanceof Node)
			// Throw when we don't support a node
			throw input.nodeName.toLowerCase()+" nodes are not supported.";
		
		// Return the input with extra data
		if (typeof(input) == 'string') {
			if (settings == undefined)
				// A string with no settings -> passthrough the string
				return input;
			else return Object.assign({'data':input},settings);
		} else return Object.assign(input,settings);
	};
	/** Parse a timestamp for the webext-adder
	 * \param input to parse
	 * \param UNIX timestamp (in seconds unlike in JS)
	 */
	static parseTimestamp(input) {
		// Integer are passed through (but not decimals)
		if (typeof(input) == 'number')
			return Math.round(input);
		else if (input instanceof Date)
			return Math.round(input.getTime()/1000);
		else if (typeof(input) == 'string')
			return Math.round(Date.parse(input)/1000);
		else throw "Unrecognized timestamp.";
	}
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
	
	/** Generate a xxx_tag_links from the list and tags list
	 * \param xxx array in it's final form
	 * \param xxx_idname is the name of the *id* field (`"source"` on artworks)
	 * \param xxx_linkid is the name of the link item id (`"artwork"` on artworks)
	 * \param tags array in it's final form
	 * \return The `xxx_tag_links` array
	 *
	 * Template function for most case of simple_xxx_tag_links functions.
	 * \see Arcollect.simple_art_tag_links that wrap this function for a practical
	 *      example.
	 */
	static simple_xxx_tag_links(xxx,xxx_idname,xxx_linkid,tags) {
		// TODO Use a flat()/flatMap() when requiring Firefox 62
		return [].concat(...(tags.map(tag => {return Object.entries({
				"tag": tag.id,
			});
		})).map(link_template => xxx.map(item => Object.fromEntries([[xxx_linkid, item[xxx_idname]],...link_template]))))
	};
	/** Generate the art_tag_links from artworks and tags list
	 * \param artworks array in it's final form
	 * \param tags array in it's final form
	 * \return The `art_tag_links` array
	 *
	 * Convenience function for most case of art_tag_links when it's just linking
	 * tags to all artworks.
	 *
	 * Here an example usage below with `artwork1`, `artwork2` and a
	 *          `postingAccount` plus an array of `mentions`.
	 * \code{.js}
	 * art_tag_links = Arcollect.simple_art_tag_links([artwork1,artwork2],{
	 * 	"account": [postingAccount]
	 * 	"indesc": mentions,
	 * })
	 * \endcode
	 */
	static simple_art_tag_links = (artworks,tags) => this.simple_xxx_tag_links(artworks,'source','artwork',tags);
	/** Generate the com_tag_links from artworks and comics list
	 * \param comics array in it's final form
	 * \param tags array in it's final form
	 * \return The `com_tag_links` array
	 *
	 * Convenience function for most case of com_tag_links when it's just linking
	 * tags to all comics.
	 *
	 * \see Arcollect.simple_art_tag_links that is the same function for linking
	 * artworks to tags.
	 */
	static simple_com_tag_links = (comics,tags) => this.simple_xxx_tag_links(comics,'id','comic',tags);
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
