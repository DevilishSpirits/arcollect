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
#include <config.h>
#include <sqlite3.hpp>
#include <arcollect-db-downloads.hpp>
#include <arcollect-debug.hpp>
#include "arcollect-paths.hpp"
#include "download.hpp"
#include "json-shared-helpers.hpp"
#include <optional>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <errno.h>
const std::string Arcollect::WebextAdder::user_agent = "Arcollect/" ARCOLLECT_VERSION_STR " curl/" + std::string(curl_version_info(CURLVERSION_NOW)->version);

extern std::unique_ptr<SQLite3::sqlite3> db;

Arcollect::WebextAdder::NetworkSession::NetworkSession(Arcollect::db::downloads::Transaction &cache)
:
	easyhandle(curl_easy_init()),
	cache(cache)
{
	
}
Arcollect::WebextAdder::NetworkSession::~NetworkSession(void)
{
	curl_easy_cleanup(easyhandle);
}

void Arcollect::WebextAdder::Download::parse(char*& iter, char* const end, Arcollect::json::ObjHave have)
{
	cache_miss = false;
	switch (have) {
		case Arcollect::json::ObjHave::STRING: {
			json_read_string_nul_terminate(have,data_string,"download URL",iter,end);
		} break;
		case Arcollect::json::ObjHave::OBJECT: {
			enum class DownloadSpec {
				data,
				cache_key,
				mimetype,
				ok_codes,
				redirection_count,
				headers,
			};
			static const ForEachObjectSwitch<DownloadSpec> downloadspec_switch{
				{"data"     ,DownloadSpec::data},
				{"cache_key",DownloadSpec::cache_key},
				{"mimetype" ,DownloadSpec::mimetype},
				{"ok_codes" ,DownloadSpec::ok_codes},
				{"redirection_count" ,DownloadSpec::redirection_count},
				{"headers"  ,DownloadSpec::headers},
			};
			for (auto entry: downloadspec_switch(iter,end))
				switch (entry.key) {
					case DownloadSpec::data: {
						json_read_string_nul_terminate(entry.have,data_string,"<download_spec>:[{\"data\"",iter,end);
					} break;
					case DownloadSpec::cache_key: {
						json_read_string(entry.have,cache_key,"<download_spec>:[{\"cache_key\"",iter,end);
						cache_miss = false;
					} break;
					case DownloadSpec::mimetype: {
						json_read_string(entry.have,mimetype,"<download_spec>:[{\"mimetype\"",iter,end);
					} break;
					case DownloadSpec::ok_codes: {
						if (entry.have != Arcollect::json::ObjHave::ARRAY)
							throw std::runtime_error("<download_spec>:[{\"ok_codes\" must be an array of integers.");
						ok_codes.clear();
						for (Arcollect::json::ArrHave have: Arcollect::json::Array(iter,end))
							ok_codes.push_back(json_read_int(have,"<download_spec>:[{\"ok_codes\":[ item",iter,end));
					} break;
					case DownloadSpec::redirection_count: {
						redirection_count = json_read_int(entry.have,"<download_spec>:[{\"redirection_count\"",iter,end);
					} break;
					case DownloadSpec::headers: {
						if (entry.have != Arcollect::json::ObjHave::OBJECT)
							throw std::runtime_error("<download_spec>:[{\"headers\" must be an object of string or null.");
						std::string_view header_name;
						bool has_header = true;
						while (has_header) {
							switch (Arcollect::json::read_object_keyval(header_name,iter,end)) {
								case Arcollect::json::ObjHave::STRING: {
									std::string_view header_value;
									json_read_string(Arcollect::json::ObjHave::STRING,header_value,"<download_spec>:[{\"headers\": \"\"",iter,end);
									http_headers.append_header(header_name,header_value);
								} break;
								case Arcollect::json::ObjHave::NULL_LITTERALLY: {
									http_headers.disable_header(header_name);
								} break;
								case Arcollect::json::ObjHave::OBJECT_CLOSE: {
									has_header = false;
								} break;
								default: {
									throw std::runtime_error("<download_spec>:[{\"headers\": elements must be strings or null."); 
								} break;
							}
						}
					} break;
				}
		} break;
		default:
			throw std::runtime_error("Invalid type for the download_spec (must be a string or an object)");
	}
	// Check URI type
	uri_type = data_string.starts_with("https://") ? URI_HTTPS : data_string.starts_with("data:") ? URI_DATA : /* Invalid scheme -> */ URI_BASE64;
	switch (uri_type) {
		case URI_HTTPS: {
			if (!cache_miss && cache_key.empty())
				// cache_key unset, use the URL as key
				cache_key = data_string;
		} break;
		case URI_DATA: {
			// Read MIME-type
			data_string = data_string.substr(5); // Erase the 'data:'
			auto comma_pos = data_string.find_first_of(',');
			std::string_view uri_mime = data_string.substr(0,comma_pos);
			// Check for base64 encoding
			if (uri_mime.ends_with(";base64")) {
				uri_type = URI_BASE64;
				uri_mime = uri_mime.substr(0,uri_mime.size()-7); // Erase the ';base64'
			}
			// Set MIME-type
			if (mimetype.empty())
				mimetype = uri_mime;
			// Update the data_string
			data_string = data_string.substr(comma_pos+1);
		} break;
		case URI_BASE64: {
			// Fail and don't support the original format
			throw std::runtime_error(std::string("Unsupported URL scheme : ")+std::string(data_string));
		} break;
	}
}
size_t Arcollect::WebextAdder::Download::curl_first_write_callback_wrapper(char *ptr, size_t size, size_t nmemb, Download *self) noexcept
{
	return self->curl_first_write_callback(ptr,size,nmemb);
}
bool Arcollect::WebextAdder::Download::assert_http_status(void)
{
	// Get HTTP status code
	long http_code;
	CURLcode curl_res = curl_easy_getinfo(session.easyhandle,CURLINFO_RESPONSE_CODE,&http_code);
	if (curl_res) {
		if (!session.curl_errorbuffer[0])
			std::memcpy(session.curl_errorbuffer2,session.curl_errorbuffer,sizeof(session.curl_errorbuffer2));
		else std::memcpy(session.curl_errorbuffer2,curl_easy_strerror(curl_res),sizeof(session.curl_errorbuffer2));
		return -1;
	}
	// Check for 304 Not Modified
	if (!cache_miss && (http_code == 304)) {
		// Signal a 304 by clearing ok_codes
		ok_codes.clear();
		return true;
	}
	// Check HTTP status code
	for (auto ok_code: ok_codes)
		if (http_code == ok_code)
			return false;
	// Failure
	std::snprintf(session.curl_errorbuffer2,sizeof(session.curl_errorbuffer2),"Bad HTTP status %ld.", http_code);
	return true;
}
size_t Arcollect::WebextAdder::Download::curl_first_write_callback(char *ptr, size_t size, size_t nmemb) noexcept
{
	CURL *const easyhandle = session.easyhandle;
	auto &curl_errorbuffer = session.curl_errorbuffer2;
	// Assert status
	if (assert_http_status())
		return -1;
	// Fill MIME-type
	if (mimetype.empty()) {
		char* curl_mimetype;
		CURLcode curl_res = curl_easy_getinfo(easyhandle,CURLINFO_CONTENT_TYPE,&curl_mimetype);
		if (curl_res)
			if (!curl_errorbuffer[0]) {
				std::strncpy(curl_errorbuffer,curl_easy_strerror(curl_res),sizeof(curl_errorbuffer));
				return -1;
			}
		if (curl_mimetype)
			// CURL hold the string long enough
			mimetype = std::string_view(curl_mimetype);
		else {
			snprintf(curl_errorbuffer,sizeof(curl_errorbuffer),"MIME-type information missing.");
			return -1;
		}
	}
	// Download is ok, perform cache transaction
	download_infos.dwn_lastedit = std::time(NULL);
	download_infos.dwn_etag = std::string_view(); // TODO Support Etag
	std::string error = session.cache.write_cache(cache_key,mimetype,download_infos);
	if (!error.empty()) {
		snprintf(curl_errorbuffer,sizeof(curl_errorbuffer),"Failed to perform transaction: %s",error.c_str());
		return -1;
	}
	// Make output file
	file = fopen((Arcollect::path::arco_data_home/download_infos.dwn_path()).string().c_str(),"wb");
	if (!file) {
		snprintf(curl_errorbuffer,sizeof(curl_errorbuffer),"Failed to perform transaction: %s",strerror(errno));
		return -1;
	}
	// Tell curl to write in our file
	if (Arcollect::debug.webext_adder)
		std::cerr << ": downloading...";
	curl_easy_setopt(easyhandle,CURLOPT_WRITEFUNCTION,fwrite);
	curl_easy_setopt(easyhandle,CURLOPT_WRITEDATA,file);
	return fwrite(ptr,size,nmemb,file);
}
sqlite_int64 Arcollect::WebextAdder::Download::perform(const std::filesystem::path& target_dir, const std::string_view& target, const std::string_view &referer)
{
	using Arcollect::db::downloads::DownloadInfo;
	if (Arcollect::debug.webext_adder)
		std::cerr << "\t\t" << data_string;
	// Check cache
	auto url_cache_iter = session.url_cache.find(cache_key);
	if (url_cache_iter != session.url_cache.end()) {
		if (Arcollect::debug.webext_adder)
			std::cerr << ": session cache hit." << std::endl;
		return url_cache_iter->second;
	}
	
	if (!cache_key.empty()) {
		// Search in the database
		download_infos = session.cache.query_cache(cache_key);
		cache_miss = !download_infos;
		if (!cache_miss && Arcollect::debug.webext_adder)
			std::cerr << ": collection cache hit";
	}
	download_infos.set_dwn_path(target_dir,target);
	// Perform transfer
	switch (uri_type) {
		case URI_HTTPS: {
			if (ok_codes.empty())
				throw std::runtime_error("<download_spec>:[{\"ok_codes\": must not be '[]' (empty array)!");
			// Configure CURL
			CURL *const easyhandle = session.easyhandle;
			curl_easy_reset(easyhandle);
			curl_easy_setopt(easyhandle,CURLOPT_URL,data_string.data());
			curl_easy_setopt(easyhandle,CURLOPT_WRITEFUNCTION,curl_first_write_callback_wrapper);
			curl_easy_setopt(easyhandle,CURLOPT_WRITEDATA,this);
			curl_easy_setopt(easyhandle,CURLOPT_PROTOCOLS,CURLPROTO_HTTPS);
			curl_easy_setopt(easyhandle,CURLOPT_REFERER,referer.data());
			curl_easy_setopt(easyhandle,CURLOPT_USERAGENT,Arcollect::WebextAdder::user_agent.c_str());
			curl_easy_setopt(easyhandle,CURLOPT_ERRORBUFFER,session.curl_errorbuffer);
			curl_easy_setopt(easyhandle,CURLOPT_LOW_SPEED_LIMIT,1L); // Abort if less than 1bytes/s
			curl_easy_setopt(easyhandle,CURLOPT_LOW_SPEED_TIME,90L); // ... for 90 seconds.
			curl_easy_setopt(easyhandle,CURLOPT_ACCEPT_ENCODING,""); // To send "Accept-Encoding:" header
			curl_easy_setopt(easyhandle,CURLOPT_SSLVERSION,CURL_SSLVERSION_TLSv1_2); 
			curl_easy_setopt(easyhandle,CURLOPT_SSL_VERIFYPEER,true); // Should be already on by default
			curl_easy_setopt(easyhandle,CURLOPT_SSL_VERIFYHOST,2); // Should be already on by default
			curl_easy_setopt(easyhandle,CURLOPT_FOLLOWLOCATION,(long)redirection_count > 0);
			curl_easy_setopt(easyhandle,CURLOPT_MAXREDIRS,redirection_count);
			curl_easy_setopt(easyhandle,CURLOPT_HTTPHEADER,http_headers.list);
			if (!cache_miss) {
				curl_easy_setopt(easyhandle,CURLOPT_TIMECONDITION,CURL_TIMECOND_IFMODSINCE); 
				curl_easy_setopt(easyhandle,CURLOPT_TIMEVALUE_LARGE,static_cast<curl_off_t>(download_infos.dwn_lastedit)); 
				// TODO Etag
			}
			// Invoke CURL
			session.curl_errorbuffer2[0] = '\0';
			CURLcode curl_res = curl_easy_perform(easyhandle);
			if (session.curl_errorbuffer2[0])
				std::memcpy(session.curl_errorbuffer,session.curl_errorbuffer2,sizeof(session.curl_errorbuffer));
			if (file)
				fclose(file);
			else if (!curl_res && !file && assert_http_status())
				// No error but no file -> no data, just check the HTTP status and if bad override curl_res
				curl_res = CURLE_WRITE_ERROR;
			// Cleanups
			switch (curl_res) {
				case CURLE_OK: {
					// Do nothing special
					if (Arcollect::debug.webext_adder)
						std::cerr << ": 200 OK." << std::endl;
					
				} return session.url_cache[cache_key] = download_infos.dwn_id();
				case CURLE_WRITE_ERROR: {
					/** Check for a 304 Not Modified
					 * In such case, assert_http_status() clear ok_codes.
					 */
					if (ok_codes.empty()) {
						if (Arcollect::debug.webext_adder)
							std::cerr << ": 304 Not Modified." << std::endl;
						return session.url_cache[cache_key] = download_infos.dwn_id();
					}
				} // falltrough;
				default: {
					if (Arcollect::debug.webext_adder)
						std::cerr << ": Failure." << std::endl;
					if (session.curl_errorbuffer[0])
						throw std::runtime_error(std::string(session.curl_errorbuffer));
					else throw std::runtime_error(curl_easy_strerror(curl_res));
				} break;
			}
		} break;
		case URI_DATA: {
			// TODO 
			throw std::runtime_error("data: URL must be encoded in Base64 for now.");
			if (mimetype.empty())
				throw std::runtime_error("A mimetype is required with data: URL.");
			if (cache_key.empty()) {
				// Write the URL encoded on-disk
				std::string error = session.cache.write_cache(cache_key,mimetype,download_infos);
				if (error.empty()) {
					std::ofstream output((Arcollect::path::arco_data_home/download_infos.dwn_path()));
					// TODO
					return session.url_cache[cache_key] = download_infos.dwn_id();
				} else throw std::runtime_error(std::string("Failed to perform transaction: ")+error);
			} else {
				// Use the cache
				return session.url_cache[cache_key] = download_infos.dwn_id();
			}
		} break;
		case URI_BASE64: {
			if (mimetype.empty())
				throw std::runtime_error("A mimetype is required when using Base64 data.");
			if (cache_key.empty()) {
				// Write the Base64 on-disk
				std::string error = session.cache.write_cache(cache_key,mimetype,download_infos);
				if (error.empty()) {
					// Decode and output Base64 data
					uint32_t current_word = 0;
					int bits_count = 0;
					std::ofstream output((Arcollect::path::arco_data_home/download_infos.dwn_path()));
					for (char digit: data_string) {
						// Compute the 6-bits word
						current_word <<= 6;
						if ((digit >= 'A')&&(digit <= 'Z'))
							current_word |= digit - 'A' + 0x00;
						else if ((digit >= 'a')&&(digit <= 'z'))
							current_word |= digit - 'a' + 0x1A;
						else if ((digit >= '0')&&(digit <= '9'))
							current_word |= digit - '0' + 0x34;
						else if (digit == '+')
							current_word |= 0x3E;
						else if (digit == '/')
							current_word |= 0x3F;
						else if (digit == '=') {
							// Skip
							current_word >>= 6; // Counter the previous shift
							continue;
						}
						else throw std::runtime_error("Invalid Base64");
						// Shift or dump
						if (bits_count == 24-6) {
							// Write bytes
							output << static_cast<uint8_t>((current_word >> 16)&0xff)
							       << static_cast<uint8_t>((current_word >>  8)&0xff)
							       << static_cast<uint8_t>((current_word >>  0)&0xff);
							bits_count = 0;
							current_word = 0;
						} else bits_count += 6;
					}
					// Print the rest of bits
					std::cerr << "Finish with " << bits_count << " bits" << std::endl;
					bits_count >>= (8-bits_count)%8;
					if (bits_count >= 16)
						output << static_cast<uint8_t>((current_word >> 8)&0xff);
					if (bits_count >= 8)
						output << static_cast<uint8_t>((current_word >> 0)&0xff);	
					// Return
					return session.url_cache[cache_key] = download_infos.dwn_id();
				} else throw std::runtime_error(std::string("Failed to perform transaction: ")+error);
			} else {
				// Use the cache
				return session.url_cache[cache_key] = download_infos.dwn_id();
			}
		} break;
	}
	throw std::runtime_error("How the fuck did I get there?");
}

std::string_view Arcollect::WebextAdder::Download::base_filename(void) const noexcept
{
	std::string_view result = cache_key;
	switch (uri_type) {
		case URI_DATA:
		case URI_BASE64: if (result.empty()) {
			// Pick 8 chars in the middle of the Base4 file
			decltype(result)::size_type pos = data_string.size()/2;
			decltype(result)::size_type length = pos;
			if (pos > 8)
				pos = 8;
			result = data_string.substr(pos,length);
		} // falltrough;
		case URI_HTTPS: {
			// If cache_key is not set, use URL
			if (result.empty())
				result = data_string;
			// If empty, return nothing
			if (result.empty())
				return result;
			// Cut at the last '/'
			decltype(result)::size_type pos;
			pos = result.find_last_of('/');
			if (pos != result.npos)
				result = result.substr(pos,result.size()-pos);
			// Cut at the first '?'
			pos = result.find_first_of('?');
			if (pos != result.npos)
				result = result.substr(0,-pos);
			// Truncate the filename at 96 chars
			if (result.size() > 96)
				result = result.substr(0,96);
		} break;
	}
	// Return
	return result;
}
