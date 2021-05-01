#include <sqlite3.hpp>
namespace Arcollect {
	extern std::unique_ptr<SQLite3::sqlite3> database;
	/** Database data_version
	 * 
	 * This is a cached value of SQLite's `PRAGMA data_version;`. It is used to
	 * detect database changes and invalidate data when required.
	 *
	 * This value is updated by update_data_version().
	 */
	extern sqlite_int64 data_version;
	
	/** Update #data_version
	 * 
	 * `PRAGMA data_version;`, return and store the result in #data_version.
	 *
	 * This function is regulary called.
	 */
	sqlite_int64 update_data_version(void);
}
