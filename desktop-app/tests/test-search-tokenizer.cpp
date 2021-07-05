#include "../db/artwork-loader.hpp"
#include "../db/search.hpp"
#include <iostream>
#include <sstream>
#include <utility>

extern SDL::Renderer *renderer;
SDL::Renderer *renderer;

using namespace Arcollect::db::search; // To avoid very long namespace writes...

#define BLANK(blank)           {TOK_BLANK     ,blank}
#define NEGATE(negate)         {TOK_NEGATE    ,negate}
#define IDENTIFIER(identifier) {TOK_IDENTIFIER,identifier}
#define COLON(colon)           {TOK_COLON     ,colon}

static struct test_case {
	const char* test_string;
	std::vector<std::pair<Token,std::string>> tokens;
} test_cases[] = {
	{"dragon",{BLANK(""),IDENTIFIER("dragon")}},
	{"-dragon",{BLANK(""),NEGATE("-"),IDENTIFIER("dragon")}},
	{"-black-dragon",{BLANK(""),NEGATE("-"),IDENTIFIER("black"),NEGATE("-"),IDENTIFIER("dragon")}},
	{"bat digital-2d",{BLANK(""),IDENTIFIER("bat"),BLANK(" "),IDENTIFIER("digital"),NEGATE("-"),IDENTIFIER("2d")}},
	{"dragon site:artstation.com",{BLANK(""),IDENTIFIER("dragon"),BLANK(" "),IDENTIFIER("site"),COLON(":"),IDENTIFIER("artstation.com")}},
};

constexpr const auto test_case_count = sizeof(test_cases)/sizeof(test_cases[0]);

std::ostringstream token_debug;

static bool tokenizer_test_new_token_callback(Arcollect::db::search::Token token, std::string_view value, void* data)
{
	std::vector<std::pair<Token,std::string>> &tokens = *reinterpret_cast<std::vector<std::pair<Token,std::string>>*>(data);
	tokens.emplace_back(token,value);
	switch (token) {
		case TOK_IDENTIFIER: {
			token_debug << " TOK_IDENTIFIER";
		} break;
		case TOK_NEGATE: {
			token_debug << " TOK_NEGATE";
		} break;
		case TOK_COLON: {
			token_debug << " TOK_COLON";
		} break;
		case TOK_BLANK: {
			token_debug << " TOK_BLANK";
		} break;
		case TOK_EOL: {
			token_debug << " TOK_EOL";
		} break;
		case TOK_INVALID: {
			token_debug << " TOK_INVALID";
		} break;
		default: {
			token_debug << " TOK_???";
		} break;
	}
	token_debug << "(\"" << value << "\")";
	return false;
}

int main(int argc, char *argv[])
{
	std::cout << "1.." << test_case_count << std::endl;
	int test_i = 1;
	for (auto &test: test_cases) {
		// Test
		std::vector<std::pair<Token,std::string>> tokens;
		Arcollect::db::search::tokenize(test.test_string,tokenizer_test_new_token_callback,&tokens);
		if (tokens != test.tokens)
			std::cout << "not ";
		std::cout << "ok " << (test_i++) << " - Tokenize \"" << test.test_string << "\" #" << token_debug.str() << std::endl;
		token_debug.str("");
	}
	// Stop background thread
	Arcollect::db::artwork_loader::stop = true;
	Arcollect::db::artwork_loader::condition_variable.notify_one();
	Arcollect::db::artwork_loader::thread.join();
	return 0;
}
