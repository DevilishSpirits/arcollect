#include "config.hpp"
#include <INIReader.h>
#include <fstream>

Arcollect::config::Param<int> Arcollect::config::first_run(0);

void Arcollect::config::read_config(void)
{
	INIReader reader(Arcollect::path::xdg_config_home/"arcollect.ini");
	// TODO if (reader.ParseError() < 0)
	first_run.value = reader.GetInteger("arcollect","first_run",first_run.default_value);
}
void Arcollect::config::write_config(void)
{
	std::ofstream writer(Arcollect::path::xdg_config_home/"arcollect.ini");
	writer << "[arcollect]\n"
	       << "first_run=" << first_run << '\n'
	;
}
