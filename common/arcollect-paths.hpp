#include <string>
#include "../sqlite3-hpp/sqlite3.hpp"
namespace Arcollect {
	namespace db {
		/** The software data home ($XDG_DATA_HOME/arcollect/)
		 *
		 * This is were all datas are stored. Artworks and database included.
		 */
		extern const std::string data_home;
		
		/** The artwork pool location ($XDG_DATA_HOME/arcollect/artworks/)
		 */
		extern const std::string artwork_pool_path;
		
		/** The account avatar pool location ($XDG_DATA_HOME/arcollect/account-avatars/)
		 */
		extern const std::string account_avatars_path;
	}
}
