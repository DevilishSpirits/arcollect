/*
 * This is an edit of the Alexander Peslyak public-domain MD5 implementation.
 *
 * Author:
 
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 * Copyright (c) 2022 DevilishSpirits (aka D-Spirits or Luc B.), maker of
 * Arcollect.
 *
 * This software was originally written by Alexander Peslyak in 2001 and edited
 * by DevilishSpirits (aka D-Spirits or Luc B.) in 2022.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak,
 * Copyright (c) 2022 DevilishSpirits (aka D-Spirits or Luc B.) and it is hereby
 * released to the general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * See md5.c for more information.
 */
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
struct MD5_CTX {
	struct DIGEST {
		char content[16];
		constexpr char& operator[](int i) { return content[i]; }
		constexpr static char util_hexdigit(char hexa) {
			if ((hexa >= '0')&&(hexa <= '9'))
				return hexa - '0' + 0x0;
			else if ((hexa >= 'a')&&(hexa <= 'f'))
				return hexa - 'a' + 0xa;
			else if ((hexa >= 'A')&&(hexa <= 'F'))
				return hexa - 'A' + 0xA;
			else return 0;
		}
		static DIGEST from_string(const char *hexa) {
			DIGEST res;
			for (unsigned int i = 0; i < sizeof(content); ++i) {
				res[i]  = util_hexdigit(*hexa++) << 4;
				res[i] |= util_hexdigit(*hexa++);
			}
			return res;
		}
		static DIGEST from_string(const std::string_view hexa) {
			return from_string(hexa.data());
		}
		constexpr bool operator==(const DIGEST& right) const {
			for (unsigned int i = 0; i < sizeof(content); ++i) {
				if (content[i] != right.content[i])
					return false;
			}
			return true;
		}
		constexpr bool operator!=(const DIGEST& right) const {
			for (unsigned int i = 0; i < sizeof(content); ++i) {
				if (content[i] != right.content[i])
					return true;
			}
			return false;
		}
	};
	std::uint_fast32_t lo, hi;
	std::uint_fast32_t a, b, c, d;
	unsigned char buffer[64];
	std::uint_fast32_t block[16];
	MD5_CTX(void)
	: lo(0)
	, hi(0)
	, a(0x67452301)
	, b(0xefcdab89)
	, c(0x98badcfe)
	, d(0x10325476)
	{}
	void Update(const void *data, std::size_t size);
	template <typename StringViewLike>
	void Update(const StringViewLike& data) {
		return Update(data.data(),data.size()*sizeof(typename StringViewLike::value_type));
	}
	template <typename T>
	MD5_CTX operator<<(const T& data) {
		Update(data);
		return *this;
	}
	DIGEST Final(void);
	template <typename ...Args>
	static DIGEST hash(Args... args) {
		return (MD5_CTX() << ... << args).Final();
	}
};
namespace std {
	inline std::string to_string(const MD5_CTX::DIGEST& value) {
		char hex[32];
		char* iter = hex;
		for (char byte: value.content) {
			static constexpr char hex_table[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
			*iter++ = hex_table[(byte >> 4)&0xf];
			*iter++ = hex_table[(byte >> 0)&0xf];
		}
		return std::string(hex,iter);
	}
}
