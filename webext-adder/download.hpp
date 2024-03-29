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
#pragma once
#include <arcollect-db-downloads.hpp>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include "curl.hpp"
#include "wtf_json_parser.hpp"

namespace Arcollect {
	namespace WebextAdder {
		/** Arcollect webext-adder user-agent
		 *
		 * This is the user-agent of Arcollect.
		 */
		extern const std::string user_agent;
		
		/** Referrer policy enum
		 *
		 * Enumeration of referrer policies.
		 * \note Arcollect is https:// only and hence skip some policies.
		 */
		enum ReferrerPolicy {
			/** Do not send a referrer at all
			 *
			 * Used by:
			 * - `no-referrer`
			 */
			REFERRER_NEVER,
			/** Send the origin only
			 *
			 * Used by:
			 * - `origin`
			 * - `strict-origin` since we never downgrade
			 */
			REFERRER_ORIGIN_ONLY,
			/** Origin only when cross-origin
			 *
			 * Used by:
			 * - `origin-when-cross-origin`
			 * - `strict-origin-when-cross-origin` since we never downgrade
			 */
			REFERRER_ORIGIN_WHEN_CROSS_ORIGIN,
			/** Origin only on the same domain
			 *
			 * Used by:
			 * - `same-origin`
			 */
			REFERRER_SAME_ORIGIN,
			/** Full referrer
			 *
			 * Used by:
			 * - `unsafe-url`
			 * - `no-referrer-when-downgrade` since we never downgrade
			 */
			REFERRER_ALWAYS,
			/** Use the default referrer policy in the NetworkSession object
			 *
			 * Value for Arcollect::webext_adder::Download to use the
			 * Arcollect::webext_adder::NetworkSession value
			 */
			REFERRER_UNSPECIFIED,
			/** Default referrer policy
			 *
			 * The default referrer policy is `strict-origin-when-cross-origin`.
			 * \warning Keep this being the last! The code is it's numeric code to
			 *          check for unknown referrers.
			 */
			REFERRER_DEFAULT = REFERRER_ORIGIN_WHEN_CROSS_ORIGIN
		};
		/** Parse a Referrer-Policy
		 * \param policy string to parse.
		 * \return The parsed policy or REFERRER_UNSPECIFIED if unknown.
		 */
		ReferrerPolicy parse_referrer_policy(std::string_view policy);
		
		/** Apply a Referrer-Policy
		 * \param referrer in complete form
		 * \param target URL to generate the referrer to
		 * \param policy string to parse.
		 * \return The referrer
		 */
		std::string apply_referrer_policy(const curl::url &referrer, const curl::url &target, ReferrerPolicy policy);
		
		/** Persistent network session
		 *
		 * This structure survive across downloads for enhanced performances.
		 * It deduplicates downloads.
		 */
		struct NetworkSession {
			/** curl easy handle
			 */
			CURL *const easyhandle;
			/* Cache transaction
			 */
			Arcollect::db::downloads::Transaction &cache;
			/** Query cache
			 *
			 * Map previous downloads do a dwn_id to avoid useless network activity.
			 * A use case is e621.net where multiple accounts generate trafic for the
			 * very same resource.
			 */
			std::unordered_map<std::string_view,sqlite3_int64> url_cache;
			/** Application submitted premade DNS resolution
			 */
			curl::slist dns_prefill;
			/** Page referrer policy
			 *
			 * The referrer policy used by the page and as default by requests.
			 */
			ReferrerPolicy referrer_policy = REFERRER_DEFAULT;
			/** curl error buffer handle
			 *
			 * For internal use by Download::perform() and
			 * Download::curl_first_write_callback().
			 */
			char curl_errorbuffer[CURL_ERROR_SIZE];
			/** Second curl error buffer handle
			 *
			 * Like curl_errorbuffer but if not empty take predecence over it.
			 */
			char curl_errorbuffer2[CURL_ERROR_SIZE];
			NetworkSession(Arcollect::db::downloads::Transaction &cache);
			NetworkSession(const NetworkSession&) = delete;
			~NetworkSession(void);
		};
		class Download {
			private:
				/** MIME type
				 *
				 * This string may be stored by the #easyhandle and is no longer valid
				 * after perform().
				 */
				std::string_view mimetype;
				/** Referrer policy
				 *
				 * The referrer policy used for this request.
				 */
				ReferrerPolicy referrer_policy = REFERRER_UNSPECIFIED;
				/** Attached network session
				 */
				NetworkSession &session;
				/** Downloads informations
				 *
				 * For internal use by perform() and curl_first_write_callback().
				 */
				Arcollect::db::downloads::DownloadInfo download_infos;
				/** True on cache miss
				 *
				 * This value is set by parse() and curl_first_write_callback().
				 *
				 * After curl_easy_perform(), it is 
				 */
				bool cache_miss;
				/** Internal FILE handle
				 *
				 * This is for internal use by curl_first_write_callback()
				 * \note It is also used as a signal that curl_first_write_callback()
				 *       has been called at least once, aka the result is not empty.
				 */
				FILE* file = NULL;
				/** Wrapper for curl_first_write_callback() member function
				 */
				static size_t curl_first_write_callback_wrapper(char *ptr, size_t size, size_t nmemb, Download *self) noexcept;
				
				/** curl first write callback
				 *
				 * This is the first [CURLOPT_WRITEFUNCTION](https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html).
				 * It perform checks and Arcollect::db::downloads::Transaction::write_cache()
				 * then switch to a plain fwrite().
				 */
				size_t curl_first_write_callback(char *ptr, size_t size, size_t nmemb) noexcept;
				/** Assert if the HTTP request failed
				 * \return true on assert failure
				 */
				bool assert_http_status(void);
				/** URL/Base64
				 */
				std::string_view data_string;
				/** Key for cache lookup
				 */
				std::string_view cache_key;
				/** List of OK HTTP status codes
				 *
				 * \note When getting a 304 Not Modified, assert_http_status() clear
				 *       this vector to let perform() distinguish a real
				 *       #CURLE_WRITE_ERROR or just a 304.
				 */
				std::vector<long> ok_codes{200};
				/** Number of allowed redirection
				 *
				 * By default none is allowed.
				 */
				long redirection_count = 0;
				/** List of custom HTTP headers
				 */
				curl::slist http_headers;
				/** Type of the #data_string
				 */
				enum {
					/** https:// schema
					 */
					URI_HTTPS,
					/** data: schema
					 */
					URI_DATA,
					/** Base64 raw data
					 */
					URI_BASE64,
				} uri_type;
			public:
				/** Target directory
				 *
				 * The directory where the resource should be downloaded
				 */
				const std::filesystem::path& target_dir;
				/** Query the base filename
				 * \return In short the part after the last '/' and before the '?' or
				 *         nothing if base64.
				 */
				std::string_view base_filename(void) const noexcept;
				/* Parse a download specification 
				 * \param iter        JSON iterator
				 * \param end         End of the JSON
				 * \param have        The type of data we have
				 */
				void parse(char*& iter, char* const end, Arcollect::json::ObjHave have);
				/** Check if the download is empty
				 */
				constexpr bool empty(void) const {
					return data_string.empty();
				}
				/** Attempt to download a resource
				 * \param target      Target filename (will be sanitized)
				 * \param referer     Referer to use.
				 * \param cache       Cache transaction to use.
				 * \return The dwn_id, throw an exception on error.
				 * \warning `referer` must be NUL-terminated!
				 *          The code ensure that with json_read_string_nul_terminate().
				 */
				sqlite_int64 perform(const std::string_view& target, const std::string_view &referer);
				/** Create a download
				 * \param session of the current transaction
				 * \param target_dir to save the artwork to (MUST BE hardcoded to a static reference)
				 */
				Download(NetworkSession &session, const std::filesystem::path& target_dir) : session(session), target_dir(target_dir) {}
		};
	}
}
