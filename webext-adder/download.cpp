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
#include "base64.hpp"
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
				headers,
			};
			static const ForEachObjectSwitch<DownloadSpec> downloadspec_switch{
				{"data"     ,DownloadSpec::data},
				{"cache_key",DownloadSpec::cache_key},
				{"mimetype" ,DownloadSpec::mimetype},
				{"ok_codes" ,DownloadSpec::ok_codes},
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
					case DownloadSpec::headers: {
						if (entry.have != Arcollect::json::ObjHave::OBJECT)
							throw std::runtime_error("<download_spec>:[{\"headers\" must be an object of string or null.");
						std::string_view header_name;
						switch (Arcollect::json::read_object_keyval(header_name,iter,end)) {
							case Arcollect::json::ObjHave::STRING: {
								std::string_view header_value;
								json_read_string(Arcollect::json::ObjHave::STRING,header_value,"<download_spec>:[{\"headers\": \"\"",iter,end);
								http_headers.append_header(header_name,header_value);
							} break;
							case Arcollect::json::ObjHave::NULL_LITTERALLY: {
								http_headers.disable_header(header_name);
							} break;
							default: {
								throw std::runtime_error("<download_spec>:[{\"headers\": elements must be strings or null."); 
						} break;
						}
					} break;
				}
		} break;
		default:
			throw std::runtime_error("Invalid type for the download_spec (must be a string or an object)");
	}
	// Check URI type
	// Note: https_prefix is 64bits, I compare the string by casting to an int64_t
	constexpr static char https_prefix[] = {'h','t','t','p','s',':','/','/'}; // "https://" without the '\0'
	uri_type = (data_string.size() > sizeof(int64_t))&&
		(*reinterpret_cast<const int64_t*>(data_string.data()) == *reinterpret_cast<const int64_t*>(https_prefix))
		? URI_HTTPS : URI_BASE64;
	switch (uri_type) {
		case URI_HTTPS: {
			if (!cache_miss && cache_key.empty())
				// cache_key unset, use the URL as key
				cache_key = data_string;
		} break;
		case URI_BASE64: {
			// Do nothing special
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
			std::strncpy(session.curl_errorbuffer,curl_easy_strerror(curl_res),sizeof(session.curl_errorbuffer));
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
	std::snprintf(session.curl_errorbuffer,sizeof(session.curl_errorbuffer),"Bad HTTP status %ld.", http_code);
	return false;
}
size_t Arcollect::WebextAdder::Download::curl_first_write_callback(char *ptr, size_t size, size_t nmemb) noexcept
{
	CURL *const easyhandle = session.easyhandle;
	auto &curl_errorbuffer = session.curl_errorbuffer;
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
	file = fopen((Arcollect::path::arco_data_home/download_infos.dwn_path).string().c_str(),"wb");
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
sqlite_int64 Arcollect::WebextAdder::Download::perform(const std::filesystem::path& target, const std::string_view &referer)
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
	download_infos.dwn_path = target;
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
			curl_easy_setopt(easyhandle,CURLOPT_SSLVERSION,CURL_SSLVERSION_TLSv1_2); 
			curl_easy_setopt(easyhandle,CURLOPT_HEADER,http_headers.list);
			if (!cache_miss) {
				curl_easy_setopt(easyhandle,CURLOPT_TIMECONDITION,CURL_TIMECOND_IFMODSINCE); 
				curl_easy_setopt(easyhandle,CURLOPT_TIMEVALUE_LARGE,static_cast<curl_off_t>(download_infos.dwn_lastedit)); 
				// TODO Etag
			}
			// Invoke CURL
			CURLcode curl_res = curl_easy_perform(easyhandle);
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
		case URI_BASE64: {
			if (mimetype.empty())
				throw std::runtime_error("A mimetype is required when using Base64 data.");
			if (cache_key.empty()) {
				// Write the Base64 on-disk
				std::string error = session.cache.write_cache(cache_key,mimetype,download_infos);
				if (error.empty()) {
					std::string binary;
					macaron::Base64::Decode(data_string.data(),binary);
					std::ofstream((Arcollect::path::arco_data_home/download_infos.dwn_path)) << binary;
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
