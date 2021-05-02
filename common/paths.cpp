#include <arcollect-paths.hpp>
#include <cstdlib>
#include <sys/stat.h>
#include <filesystem>
const std::string Arcollect::db::data_home = [](){
	// Compute the data home
	std::filesystem::path data_home;
	if (auto* xdg_home = std::getenv("XDG_DATA_HOME"); xdg_home && *xdg_home != '\0')
		data_home = std::filesystem::path(xdg_home) / "arcollect";
	else
		data_home = std::filesystem::path(std::getenv("HOME")) / ".local" / "share" / "arcollect" / "";
	// Create the directory if it does not exist
	std::filesystem::create_directory(data_home);
	
	return data_home;
}();

const std::string Arcollect::db::artwork_pool_path = [](){
	// Compute the data home
	std::filesystem::path stringartwork_pool_path = data_home + "/artworks/";
	// Create the directory if it does not exist
	std::filesystem::create_directory(stringartwork_pool_path);
	
	return stringartwork_pool_path;
}();

const std::string Arcollect::db::account_avatars_path = [](){
	// Compute the data home
	std::filesystem::path stringaccount_avatars_path = data_home + "/account-avatars/";
	// Create the directory if it does not exist
	std::filesystem::create_directory(stringaccount_avatars_path);
	
	return stringaccount_avatars_path;
}();
