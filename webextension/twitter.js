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
/** \file webextension/twitter.js
 *  \brief Twitter background script helper
 *
 * Twitter HTML is awful and hard to parse, we listen to API calls insteads and
 * that must be done in the background scripts.
 * \todo Gradually clear caches.
 * \todo Obey to `tweet['display_text_range']` (it's UTF-32 indexes...)
 */
var twitter_tweets_cache = {}
var twitter_accounts_cache = {}
var twitter_debug = false;

function twitter_handle_tweet(tweet) {
	// For retweets, save the source tweets
	if (tweet.hasOwnProperty('retweeted_status_result'))
		tweet = tweet.retweeted_status_result.result.legacy
	// Ensure that there is medias before processing the tweet
	let entities = tweet['entities'];
	if (entities.hasOwnProperty('media')) {
		let media = entities['media'];
		if (media.length) {
			// Cache relevant tweet infos
			let id = parseInt(tweet['id_str'])
			let user_mentions = []
			if (entities.hasOwnProperty('user_mentions'))
				entities['user_mentions'].forEach(mention => user_mentions.push(parseInt(mention.id_str)));
			let hashtags = []
			if (entities.hasOwnProperty('hashtags'))
				entities['hashtags'].forEach(hashtag => hashtags.push(hashtag.text));
			let medias = []
			entities['media'].forEach(pic => medias.push(pic.media_url_https));
			twitter_tweets_cache[id] = {
				'artwork_template': Object.entries({
					"title": tweet['full_text'].split('\n')[0], // TODO Obey to tweet['display_text_range'] >>UTF-32<< delimitations (RAAAH!!!)
					"description": tweet['full_text'],
					"rating": tweet['possibly_sensitive'] ? 18 : 0,
					"postdate": Date.parse(tweet['created_at'])/1000,
				}),
				'tweet_account' : parseInt(tweet['user_id_str']),
				'mention_accounts': user_mentions,
				'hashtags' : hashtags,
				'medias': medias,
			}
			if (twitter_debug)
				console.debug('Found Twitter tweet',id,':',twitter_tweets_cache[id]);
		}
	}
}
function twitter_handle_user(user) {
	if (user.hasOwnProperty('rest_id')) {
		user.legacy['id'] = user.rest_id;
		user = user.legacy;
	}
	// Search for user avatar
	let icon_url = null;
	const image_url_properties = [
		'profile_image_url_https',
		'avatar_image_url',
	];
	for (i in image_url_properties)
		if (user.hasOwnProperty(image_url_properties[i])) {
		icon_url = user[image_url_properties[i]];
		break;
	}
	if (icon_url == null)
		console.error('Failed to extract avatar from this account',user);
	// Search for user id
	let id = null;
	const id_properties = [
		'id',
		'id_str',
		'user_id',
	];
	for (i in id_properties)
		if (user.hasOwnProperty(id_properties[i])) {
		id = parseInt(user[id_properties[i]]);
		break;
	}
	if (id == null)
		console.error('Failed to extract user id from this account',user);
	// Create account entry
	if ((icon_url != null)&&(id != null)) {
		twitter_accounts_cache[id] = {
			"id": id,
			"name": user['screen_name'],
			"title": user['name'],
			"url": "https://twitter.com/"+user['screen_name'],
			"icon": icon_url.replace('http://','https://').replace('_normal.','_400x400.')
		};
		if (twitter_debug)
			console.debug('Found Twitter account',twitter_accounts_cache[id]);
	};
}
function twitter_handle_users(users) {
	Object.entries(users).forEach((user) => twitter_handle_user(user[1]));
}
function twitter_handle_itemContent(itemContent) {
	if (itemContent['itemType'] == 'TimelineTweet') {
		let tweet_results = itemContent['tweet_results']
		if (tweet_results.hasOwnProperty('result')) {
			twitter_handle_tweet(tweet_results.result.legacy);
			// Handle users in the tweet
			twitter_handle_user(tweet_results.result.core.user_results.result);
		}
	}
}
function twitter_handle_items(items) {
	items.forEach(function(entry) {
		if (entry.hasOwnProperty('item')) {
			twitter_handle_itemContent(entry['item']['itemContent']);
		}
		if (entry.hasOwnProperty('items')) {
			twitter_handle_items(entry['items']);
		}
	});
}
function twitter_handle_entries(entries) {
	entries.forEach(function(entry) {
		let content = entry['content'];
		if (content.hasOwnProperty('itemContent')) {
			twitter_handle_itemContent(content['itemContent'])
		}
		if (content.hasOwnProperty('items')) {
			twitter_handle_items(content['items']);
		}
	});
}
function twitter_handle_globalObjects(globalObjects) {
	if (globalObjects.hasOwnProperty('tweets'))
		Object.entries(globalObjects.tweets).forEach((tweet) => twitter_handle_tweet(tweet[1]));
	if (globalObjects.hasOwnProperty('users'))
		twitter_handle_users(globalObjects.users);
}
function twitter_handle_instructions(instructions) {
	instructions.forEach(function(instruction) {
	if (instruction.hasOwnProperty('entries'))
			twitter_handle_entries(instruction['entries'])
	else if (instruction.hasOwnProperty('addEntries'))
		twitter_handle_entries(instruction['addEntries']['entries'])
	});
}
var twitter_running_requests = {}
const twitter_json_text_decoder = new TextDecoder('utf-8')
function twitter_intercept_api_call(requestId) {
	let filter = browser.webRequest.filterResponseData(requestId);
	twitter_running_requests[requestId] = [];
	filter.ondata = function(event) {
		twitter_running_requests[requestId].push(twitter_json_text_decoder.decode(event.data));
		filter.write(event.data);
	}
	filter.onstop = function(event) {
		filter.close();
		if (twitter_running_requests[requestId].length) {
			// Reassemble and parse the JSON
			let api_json = JSON.parse(twitter_running_requests[requestId].join(''));
			// Handle the JSON
			if (api_json.hasOwnProperty('globalObjects'))
				twitter_handle_globalObjects(api_json['globalObjects']);
			if (api_json.hasOwnProperty('data')) {
				let data = api_json['data'];
				if (data.hasOwnProperty('user')) {
					let result = data.user.result;
					if (result.hasOwnProperty('data'))
						twitter_handle_user(result.data);
					if (result.hasOwnProperty('timeline'))
						twitter_handle_instructions(result.timeline.timeline.instructions);
					if (result.hasOwnProperty('legacy')) {
						twitter_handle_user(result);
					}
				}
				if (data.hasOwnProperty('users')) {
					Object.entries(data.users).forEach((user) => twitter_handle_user(user[1].result));
				}
				if (data.hasOwnProperty('threaded_conversation_with_injections')) {
					twitter_handle_instructions(data.threaded_conversation_with_injections.instructions)
				}
			}
			if (api_json.hasOwnProperty('users'))
				twitter_handle_users(api_json['users']);
			if (api_json.hasOwnProperty('timeline'))
				twitter_handle_instructions(api_json.timeline.instructions);
		}
		delete twitter_running_requests[requestId];
	}
}

browser.webRequest.onBeforeRequest.addListener(function(details) {
	// Filter the request
	if ((details.method == 'GET')&&(details.url.match('.+/api/.+')))
		twitter_intercept_api_call(details.requestId);
},{urls:["https://twitter.com/*","https://mobile.twitter.com/*"], types: ["xmlhttprequest"]}, ["blocking"]);

/** Process a content-script request
 * \param json The request of the content-script
 * \return A complete JSON for webext-adder, or null on error.
 *
 * \note On error, the function call reply_to_content_script() itself.
 */
function twitter_post_process_submit(json) {
	let account_ids = new Set();
	let hashtags = new Set();
	json.art_acc_links = []
	// Process artworks
	if (json.hasOwnProperty('artworks'))
		for (i in json['artworks']) {
			// ["https:","","twitter.com","<user>","status","<id>","photo","<pic_no>"]
			let source = json['artworks'][i]['source'];
			let source_splited = source.split('/');
			let id = parseInt(source_splited[5]);
			let pic_no = parseInt(source_splited[7]);
			const cached_tweet = twitter_tweets_cache[id];
			if (cached_tweet == null) {
				console.warn('Tweet',id,'is not in cache');
				reply_to_content_script({'transaction_id': json.transaction_id, 'success': false, 'reason': browser.i18n.getMessage('webext_tweet_not_in_cache')});
				return null;
			}
			// Set data
			json['artworks'][i]['data'] = cached_tweet.medias[pic_no-1]+'?name=orig';
			// Copy artwork_template
			cached_tweet.artwork_template.forEach(pair => json['artworks'][i][pair[0]] = pair[1]);
			// Add accounts infos
			account_ids.add(cached_tweet.tweet_account);
			json.art_acc_links.push({'account': cached_tweet.tweet_account, 'artwork': source, 'link': 'account'})
			cached_tweet.mention_accounts.forEach(function(user_id) {
				account_ids.add(user_id);
				json.art_acc_links.push({'account': user_id, 'artwork': source, 'link': 'indesc'});
			});
			// Add hashtags infos
			cached_tweet.hashtags.forEach(hashtag => hashtags.add(hashtag));
		}
	
	// Generate account infos
	json['accounts'] = []
	for (id of account_ids.keys()) {
		const cached_account = twitter_accounts_cache[id];
		if (cached_account == null) {
				console.warn('Twitter account',id,'is not in cache');
				reply_to_content_script({'transaction_id': json.transaction_id, 'success': false, 'reason': browser.i18n.getMessage('webext_twitter_account_not_in_cache')});
				return null;
		} else json.accounts.push(cached_account);
	}
	return json;
}
