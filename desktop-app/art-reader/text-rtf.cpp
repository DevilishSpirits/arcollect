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
/** \file desktop-app/art-reader/text-rtf.cpp
 *  \brief RichTextFormat artwork reader
 *
 * This parser is extremely simplistic, permissive and incomplete.
 * It is based on the RTF 1.9.1 specification.
 */
#include "text.hpp"
#include <arcollect-debug.hpp>
#include <charconv>
#include <optional>
#include <system_error>
struct ControlWord {
	std::string_view command;
	std::optional<int> param;
	static constexpr bool is_valid_command_char(char chr) {
		return ((chr <= 'z')&&(chr >= 'a')) || ((chr <= 'Z')&&(chr >= 'A'));
	}
	void param_set_default(int value) {
		if (!param)
			param = value;
	}
	ControlWord(const char*& iter, const char *const end)
	{
		// Read the command name
		const char* start = iter;
		for (;(iter != end) && is_valid_command_char(*iter); ++iter);
		command = std::string_view(start,std::distance(start,iter));
		start = iter;
		// Read the numeric value if so
		if (((*iter >= '0')&(*iter <= '9')) || (*iter == '-')) {
			std::from_chars_result parse_result = std::from_chars(iter,end,param.emplace());
			if (parse_result.ec != std::errc())
				throw std::system_error(std::make_error_code(parse_result.ec));
			iter = parse_result.ptr;
		}
		// Skip the space after control word
		if ((iter != end) && (*iter == ' '))
			// Space after control word -> part of the control word so skip it
			++iter;
	}
};
struct RTFGroupState;
struct RTFGlobalState {
	Arcollect::art_reader::TextElements main_elements;
};

using RTFCommand     = std::function<void(ControlWord,RTFGroupState&)>;
using RTFCommandSet  = std::unordered_map<std::string_view,RTFCommand>;
using RTFTextHandler = std::function<void(const std::string_view&,RTFGroupState&, bool utf8)>;

using WindowsHCodepage = std::u32string_view;
static const char32_t default_hcodepage[128] = U"                                                                                                                               ";
static const std::unordered_map<int,WindowsHCodepage> windows_hcodepages{
	{1252,U"€ ‚ƒ„…†‡ˆ‰Š‹Œ Ž  ‘’“”•–—˜™š›œ žŸ\x00A0¡¢£¤¥¦§¨©ª«¬ ®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ",
	},
};
struct RTFGroupState {
	RTFGlobalState& global;
	RTFTextHandler text_handler;
	RTFCommandSet command_set;
	RTFCommand unknow_command;
	const char32_t *hcodepage;
};

static void rtf_echo_text(const std::string_view& text, RTFGroupState& state, bool utf8)
{
	if (utf8)
		state.global.main_elements << text;
	else {
		// Map non-ASCII chars to Unicode equivalent
		const char *current = text.data();
		std::size_t remaining = text.size();
		for (std::size_t i = 0; i < remaining; ++i) {
			unsigned char byte = reinterpret_cast<const unsigned char&>(current[i]);
			if (byte >= 0x80) {
				// Print special character
				state.global.main_elements << std::string_view(current,i) << std::u32string_view(&state.hcodepage[byte-0x80],1);
				// Reset counters
				current += ++i;
				remaining -= i;
				i = 0;
			}
		}
		state.global.main_elements << std::string_view(current,remaining);
	}
}
static void rtf_skip_text(const std::string_view& text, RTFGroupState& state, bool utf8)
{
	// Do nothing
}

static void rtf_command_noop(ControlWord control_word, RTFGroupState& state)
{
}
static void rtf_skip_command_group(ControlWord control_word, RTFGroupState& state)
{
	state.command_set.clear();
	state.text_handler = rtf_skip_text;
	state.unknow_command = rtf_command_noop;
}

static void rtf_unsupported_charset(ControlWord control_word, RTFGroupState& state)
{
	throw std::runtime_error("Only \\ansi encoding is supported, document use \\"s+std::string(control_word.command));
}
static void rtf_unknow_command(ControlWord control_word, RTFGroupState& state)
{
	if (Arcollect::debug.rtf) {
		state.global.main_elements << SDL::Color(0,255,0,255);
		state.text_handler("\\",state,true);
		state.text_handler(control_word.command,state,true);
		if (control_word.param) {
			state.global.main_elements << SDL::Color(0,0,255,255);
			state.text_handler(std::to_string(*control_word.param),state,true);
		}
		state.text_handler(" ",state,true);
		state.global.main_elements << SDL::Color(255,255,255,255);
	}
}
struct rtf_put_chars {
	const std::string_view chars;
	void operator()(ControlWord control_word, RTFGroupState& state) {
		state.text_handler(chars,state,true);
	}
	rtf_put_chars(const std::string_view& chars) : chars(chars) {}
};

static RTFCommandSet main_command_set{
	// Character sets
	{"ansi",rtf_command_noop},
	{"mac" ,rtf_unsupported_charset},
	{"pc"  ,rtf_unsupported_charset},
	{"pca" ,rtf_unsupported_charset},
	{"ansicpg",[](ControlWord control_word, RTFGroupState& state) {
		if (control_word.param) {
			auto iter = windows_hcodepages.find(*control_word.param);
			if (iter != windows_hcodepages.end())
				state.hcodepage = iter->second.data();
		}
	}},
	// Header commands
	{"fonttbl",rtf_skip_command_group},
	{"colortbl",rtf_skip_command_group},
	{"stylesheet",rtf_skip_command_group},
	// Document commands
	{"info",rtf_skip_command_group},
	// Escapes
	{"bullet",rtf_put_chars("•")},
	{"ldblquote",rtf_put_chars("“")},
	{"rdblquote",rtf_put_chars("”")},
	{"par",rtf_put_chars("\n\n")},
	{"line",rtf_put_chars("\n")},
	{"lquote",rtf_put_chars("‘")},
	{"rquote",rtf_put_chars("’")},
	};
static RTFCommandSet start_command_set{
	{"rtf", [](ControlWord control_word, RTFGroupState& state) {
		if (!control_word.param)
			throw std::runtime_error("\\rtf must have a version (Arcollect only support 1.x versions)");
		if (control_word.param != 1)
			throw std::runtime_error("RTF version "s+std::to_string(*control_word.param)+" not supported (Arcollect only support 1.x versions)");
		// Version is okay
		state.command_set = main_command_set;
		state.unknow_command = rtf_unknow_command;
	}},
};
Arcollect::art_reader::TextElements Arcollect::art_reader::text_rtf(const char* iter, const char *const end)
{
	try {
		RTFGlobalState global;
		std::vector<RTFGroupState> group_stack{{
			// Head of the stack
			global,
			rtf_echo_text,
			start_command_set,
			[](ControlWord, RTFGroupState& state) {
				throw std::runtime_error("RTF documents must begin with an \\rtf1 command");
			},
			default_hcodepage,
		}};
		for (;iter != end; ++iter) {
			RTFGroupState &state = group_stack.back();
			switch (*iter) {
				case '\\': {
					// Process command
					if (++iter == end)
						break;
					switch (*iter) {
						case '*': {
							// Ignore next commands
							state.text_handler = rtf_skip_text;
						} break;
						case '\'': {
							if (++iter == end)
								break;
							// Hexa escape
							unsigned char character;
							std::from_chars_result parse_result = std::from_chars(iter,end,character,16);
							if (parse_result.ec != std::errc())
								throw std::system_error(std::make_error_code(parse_result.ec));
							// Skip the hexa
							++iter;
							++iter;
							if (parse_result.ptr != iter)
								// Parsing went wrong
								return Arcollect::art_reader::TextElements::build(U"Invalid hexa in \\'xx sequence"sv);
							state.text_handler(std::string_view(&reinterpret_cast<const char&>(character),1),state,false);
							--iter;
						} break;
						case '\\':case '{':case '}': {
							// Print escaped character as is
							state.text_handler(std::string_view(iter,1),state,true);
						} break;
						default: {
							ControlWord control_word(iter,end);
							auto cmd_iter = state.command_set.find(control_word.command);
							if (cmd_iter != state.command_set.end())
								cmd_iter->second(control_word,state);
							else state.unknow_command(control_word,state);
							--iter;
						} break;
					}
				} break;
				case '{': {
					// Push a copy of the current state
					if (Arcollect::debug.rtf) {
						global.main_elements << SDL::Color(255,0,0,255);
						state.text_handler("{",state,true);
						global.main_elements << SDL::Color(255,255,255,255);
					}
					group_stack.push_back(state);
				} break;
				case '}': {
					group_stack.pop_back();
					// Pop state from stack
					if (Arcollect::debug.rtf) {
						global.main_elements << SDL::Color(255,255,0,255);
						group_stack.back().text_handler("}",group_stack.back(),true);
						global.main_elements << SDL::Color(255,255,255,255);
					}
					if (group_stack.empty())
						return Arcollect::art_reader::TextElements::build(U"Mismatched '}' in RTF file"sv);
				} break;
				case '\r':case '\n': {
					// Skip line breaks
				} break;
				default: {
					// Process text
					// TODO Do bulk addition, no individual char
					state.text_handler(std::string_view(iter,1),state,false);
				} break;
			}
		}
		return global.main_elements;
	} catch (std::exception &e) {
		return Arcollect::art_reader::TextElements::build(SDL::Color(255,0,0,255),std::string_view(e.what()));
	}
}
