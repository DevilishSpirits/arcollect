#include <arcollect-i18n-common.hpp>
#include <arcollect-i18n-webextension.hpp>
#include <iostream>
#include <unordered_set>
#include "../webext-adder/json_escaper.hpp"
using Arcollect::i18n::Lang;

static bool first_translation;
template <typename T>
static void translate_module(const T &translation)
{
	for (const std::string_view &str: translation.po_strings) {
		if (first_translation)
			first_translation = false;
		else std::cout << ",\n";
		std::cout << "\t\t\"" << str << "\": {\n\t\t\t\"message\": \"" << Arcollect::json::escape_string(translation.get_po_string(str)) << "\"\n\t\t}";
	}
}

int main(void)
{
	// List langs
	std::unordered_set<Lang> langs{"en"};
	for (Lang lang: Arcollect::i18n::common::translations)
		langs.insert(lang);
	for (Lang lang: Arcollect::i18n::webextension::translations)
		langs.insert(lang);
	
	// Generate the JSON
	std::cout << "{\n";
	bool first_lang = true;
	for (Lang lang: langs) {
		if (first_lang)
			first_lang = false;
		else std::cout << ",\n";
		
		std::cout << "\t\"" << lang.lang.to_string();
		if (lang.country)
			std::cout << '_' << lang.country.to_string();
		std::cout << "\": {\n";
		
		first_translation = true;
		Arcollect::i18n::common common_translation;
		common_translation.apply_locale(lang);
		translate_module(common_translation);
		
		Arcollect::i18n::webextension webext_translation;
		webext_translation.apply_locale(lang);
		translate_module(webext_translation);
		std::cout << "\n\t}";
	}
	std::cout << "\n}";
}
