#include <filesystem>
#include "../sqlite3-hpp/sqlite3.hpp"
namespace Arcollect {
	namespace path {
		/** The XDG configuration home ($XDG_CONFIG_HOME)
		 */
		extern const std::filesystem::path xdg_config_home;
		/** The Arcollect data home ($XDG_DATA_HOME/arcollect/)
		 *
		 * This is were all datas are stored. Artworks and database included.
		 */
		extern const std::filesystem::path arco_data_home;
		
		/** The artwork pool location ($XDG_DATA_HOME/arcollect/artworks/)
		 */
		extern const std::filesystem::path artwork_pool;
		
		/** The account avatar pool location ($XDG_DATA_HOME/arcollect/account-avatars/)
		 */
		extern const std::filesystem::path account_avatars;
	}
}
