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
/** \file test_wtf_json_parser.cpp
 *  \brief Test the WTF JSON parser
 *
 * Read a JSON and *should* ouput an identitical JSON in the output.
 */
#include "../download.hpp"
#include <iostream>
using namespace Arcollect::WebextAdder; // To shorten test length

struct test_item {
	std::string policy;
	std::string orig_referrer;
	std::string target;
	ReferrerPolicy expected_policy;
	std::string expected_referrer;
};
static const test_item test_items[] = {
	// Check that we strip the fragment
	{"unsafe-url","https://d-spirits.me/void.png?emptiness=1.0#fragment_of_nothing","https://d-spirits.me/",REFERRER_ALWAYS,"https://d-spirits.me/void.png?emptiness=1.0"},
	// Check that our default policy is "origin-when-cross-origin"
	{"","https://d-spirits.me/void.webp","https://mozilla.org",REFERRER_UNSPECIFIED,"https://d-spirits.me/"},
	// From https://developer.mozilla.org/docs/Web/HTTP/Headers/Referrer-Policy#examples
	{"no-referrer","https://example.com/page","https://example.com/otherpage",REFERRER_NEVER,""},
	{"no-referrer","https://example.com/page","https://mozilla.org"          ,REFERRER_NEVER,""},
	{"origin","https://example.com/page","https://example.com/otherpage",REFERRER_ORIGIN_ONLY,"https://example.com/"},
	{"origin","https://example.com/page","https://mozilla.org"          ,REFERRER_ORIGIN_ONLY,"https://example.com/"},
	{"origin-when-cross-origin","https://example.com/page","https://example.com/otherpage",REFERRER_ORIGIN_WHEN_CROSS_ORIGIN,"https://example.com/page"},
	{"origin-when-cross-origin","https://example.com/page","https://mozilla.org"          ,REFERRER_ORIGIN_WHEN_CROSS_ORIGIN,"https://example.com/"},
	{"strict-origin","https://example.com/page","https://example.com/otherpage",REFERRER_ORIGIN_ONLY,"https://example.com/"},
	{"strict-origin","https://example.com/page","https://mozilla.org"          ,REFERRER_ORIGIN_ONLY,"https://example.com/"},
	{"strict-origin-when-cross-origin","https://example.com/page","https://example.com/otherpage",REFERRER_ORIGIN_WHEN_CROSS_ORIGIN,"https://example.com/page"},
	{"strict-origin-when-cross-origin","https://example.com/page","https://mozilla.org"          ,REFERRER_ORIGIN_WHEN_CROSS_ORIGIN,"https://example.com/"},
	{"unsafe-url","https://example.com/page","https://example.com/otherpage",REFERRER_ALWAYS,"https://example.com/page"},
	{"unsafe-url","https://example.com/page","https://mozilla.org"          ,REFERRER_ALWAYS,"https://example.com/page"},
};

static std::ostream& operator<<(std::ostream& left, ReferrerPolicy right)
{
	switch (right) {
		#define CASE(value) case value:return left << #value;
		CASE(REFERRER_UNSPECIFIED);
		CASE(REFERRER_NEVER);
		CASE(REFERRER_ORIGIN_ONLY);
		CASE(REFERRER_ORIGIN_WHEN_CROSS_ORIGIN);
		CASE(REFERRER_SAME_ORIGIN);
		CASE(REFERRER_ALWAYS);
		default:return left << "REFERRER_UNKNOWN_" << static_cast<int>(right);
	}
}

int main(void)
{
	std::cout << "TAP version 13" << std::endl;
	std::cout << "1.." << (sizeof(test_items)/sizeof(test_items[0])) << std::endl;
	size_t test_no = 0;
	for (test_item item: test_items) {
		// Perform policy check
		ReferrerPolicy resolved_policy = parse_referrer_policy(item.policy);
		curl::url referrer_url;
		referrer_url.set(CURLUPART_URL,item.orig_referrer.c_str());
		curl::url target_url;
		target_url.set(CURLUPART_URL,item.target.c_str());
		std::string resolved_referrer = apply_referrer_policy(referrer_url,target_url,resolved_policy);
		// Print test line
		bool success = (resolved_referrer == item.expected_referrer)&&(resolved_policy == item.expected_policy);
		std::cout << (success ? "ok " : "not ok ") << ++test_no << " \""<< item.policy << "\": " << item.orig_referrer << " -> " << item.target;
		if (!success)
			std::cout << " # expected:"<<item.expected_policy<<":" << item.expected_referrer << ", got:" << resolved_policy << ":" << resolved_referrer;
		std::cout << std::endl;
	}
	
}
