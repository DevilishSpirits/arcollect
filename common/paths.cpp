#include <arcollect-paths.hpp>
#include <cstdlib>
#include <sys/stat.h>
const std::filesystem::path Arcollect::path::xdg_config_home = [](){
	// Compute the data home
	if (auto* config_home = std::getenv("XDG_CONFIG_HOME"); config_home && *config_home != '\0')
		return std::filesystem::path(config_home);
	else return std::filesystem::path(std::getenv("HOME")) / ".config";
}();

const std::filesystem::path Arcollect::path::arco_data_home = [](){
	// Compute the data home
	std::filesystem::path data_home;
	if (auto* xdg_home = std::getenv("XDG_DATA_HOME"); xdg_home && *xdg_home != '\0')
		data_home = std::filesystem::path(xdg_home) / "arcollect";
	else
		data_home = std::filesystem::path(std::getenv("HOME")) / ".local" / "share" / "arcollect";
	// Create the directory if it does not exist
	std::filesystem::create_directory(data_home);
	
	return data_home;
}();

const std::filesystem::path Arcollect::path::artwork_pool = [](){
	// Compute the data home
	std::filesystem::path stringartwork_pool_path = arco_data_home / "artworks";
	// Create the directory if it does not exist
	std::filesystem::create_directory(stringartwork_pool_path);
	
	return stringartwork_pool_path;
}();

const std::filesystem::path Arcollect::path::account_avatars = [](){
	// Compute the data home
	std::filesystem::path stringaccount_avatars_path = arco_data_home / "account-avatars";
	// Create the directory if it does not exist
	std::filesystem::create_directory(stringaccount_avatars_path);
	
	return stringaccount_avatars_path;
}();
