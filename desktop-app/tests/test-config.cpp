#include "../config.hpp"
#include "../db/artwork-loader.hpp"
#include <iostream>

#define foreach_param_step(config_name,instructions) \
{ \
	auto& param = Arcollect::config::config_name; \
	const std::string param_name = #config_name; \
	instructions\
}
#define foreach_int_param(instructions) \
{ \
	foreach_param_step(first_run,instructions); \
	foreach_param_step(start_window_mode,instructions); \
	foreach_param_step(current_rating,instructions); \
}
#define foreach_param(instructions) \
{ \
	foreach_int_param(instructions); \
}

extern SDL::Renderer *renderer;
SDL::Renderer *renderer;

int main(void)
{
	bool test_success;
	std::cout << "1..1" << std::endl;
	std::cout << "# Using configuration file at " << Arcollect::path::xdg_config_home/"arcollect.ini" << std::endl;
	
	// Check if reset to default works
	std::cout << "# Check if default values are correctly set" << std::endl;
	Arcollect::config::write_config();
	Arcollect::config::read_config();
	test_success = true;
	foreach_param({
		bool step_success = param.default_value == param;
		std::cout << "# " << param_name << " default is " << param.default_value << ", now is " << param << ", success:" << step_success << std::endl;
		test_success &= step_success;
	});
	std::cout << (test_success ? "ok" : "not ok") << " 1 - Check if settings to default values works" << std::endl;
	// Stop background thread
	Arcollect::db::artwork_loader::stop = true;
	Arcollect::db::artwork_loader::condition_variable.notify_one();
	Arcollect::db::artwork_loader::thread.join();
	return 0;
}
