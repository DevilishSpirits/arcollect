/* Usage: gen-roboto-embed.cpp db-schema.cpp arcollect-db-schema.hpp Roboto-variants...
 *
 * This is a simple helper that is not designed to work on every edge cases, I
 * took harmless liberties here. Do not report bugs unless his behavior cause
 */
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>

static std::fstream cpp;
static std::fstream hpp;

static void write_ttf_file(const char* ttf_file_path, const std::string &var_name)
{
	//std::cerr << "Including \"" << ttf_file_path << "\" as Arcollect::Roboto::" << var_name << "\n" << std::endl;
	char ttf_file[175000];
	std::ifstream ttf(ttf_file_path);
	//ttf.exceptions(ttf.failbit|ttf.badbit);
	ttf.read(ttf_file,sizeof(ttf_file));
	// Fail if not at EOF
	if (!ttf.eof()) {
		std::cerr << "EOF not reached after loading \"" << ttf_file_path << "\". Increase the length of \"ttf_file[" << sizeof(ttf_file) << "];\" around line " << __LINE__ << " of " << __FILE__ << " and try again." << std::endl;
		std::exit(1);
	}
	const auto ttf_size = ttf.gcount();
	// Start C++ variable
	hpp << "\t\textern const std::array<unsigned char," << ttf_size << "> " << var_name << ";\n";
	cpp << "const std::array<unsigned char," << ttf_size << "> Arcollect::Roboto::" << var_name << " = {";
	// Write file
	cpp << (int)static_cast<unsigned char>(ttf_file[0]);
	for (auto i = 1; i < ttf_size; i++)
		cpp << ',' << (int)static_cast<unsigned char>(ttf_file[i]);
	cpp << "};\n";
}

int main(int argc, char** argv)
{
	// Create files
	cpp.open(argv[1],std::fstream::out);
	hpp.open(argv[2],std::fstream::out);
	cpp << "/* C++ source automatically generated by " << __FILE__ << " */\n"
	"#include <arcollect-roboto.hpp>" << std::endl;
	hpp << "/* C++ header automatically generated by " << __FILE__ << " */\n"
	"#pragma once\n#include <array>\n"
	"namespace Arcollect {\n\tnamespace Roboto {"
	<< std::endl;
	// Write ttf files
	for (auto i = 3; argv[i]; i++) {
		// File names are ../../dir/dir/Roboto-var_name.ttf
		int var_name_length = -1;
		int pos;
		for (pos = std::strlen(argv[i])-4; (pos > 0) && (argv[i][pos] != '-'); pos--)
			var_name_length++;
		std::string var_name(argv[i],pos+1,var_name_length);
		write_ttf_file(argv[i],var_name);
	}
	// Finish the header
	hpp << "\t}\n}\n";
}
