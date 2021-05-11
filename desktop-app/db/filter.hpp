#include <sqlite3.hpp>
#include "../config.hpp"
namespace Arcollect {
	namespace db_filter {
		/** Version of the db_filter
		 */
		extern unsigned int version;
		/** Get the current global filter
		 */
		std::string get_sql(void);
		
		/** Set current rating
		 *
		 */
		void set_rating(config::Rating rating);
	}
}
