#include <arcollect-paths.hpp>
#include <cstdlib>
#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#endif
const std::filesystem::path Arcollect::path::xdg_config_home = [](){
	// Compute the data home
	if (auto* config_home = std::getenv("XDG_CONFIG_HOME"); config_home && *config_home != '\0')
		return std::filesystem::path(config_home);
	else {
		#if defined(_WIN32)
		// Store in Local AppData folder
		PWSTR ppszPath;
		SHGetKnownFolderPath(FOLDERID_LocalAppData,KF_FLAG_CREATE,NULL,&ppszPath); // TODO Check result
		std::filesystem::path result(ppszPath);
		CoTaskMemFree(ppszPath);
		result /= "Arcollect";
		return result;
		#else // WITH_XDG
		return std::filesystem::path(std::getenv("HOME")) / ".config";
		#endif
	}
}();

const std::filesystem::path Arcollect::path::arco_data_home = [](){
	// Compute the data home
	std::filesystem::path data_home;
	if (auto* arcollect_home = std::getenv("ARCOLLECT_DATA_HOME"); arcollect_home && *arcollect_home != '\0')
		data_home = std::filesystem::path(arcollect_home);
	else if (auto* xdg_home = std::getenv("XDG_DATA_HOME"); xdg_home && *xdg_home != '\0')
		data_home = std::filesystem::path(xdg_home) / "arcollect";
	else {
		#if defined(_WIN32)
		// Store in local AppData folder (same dir as config)
		data_home = Arcollect::path::xdg_config_home;
		#else // WITH_XDG
		data_home = std::filesystem::path(std::getenv("HOME")) / ".local" / "share" / "arcollect";
		#endif
	}
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
