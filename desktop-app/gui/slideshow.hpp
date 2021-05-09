/** \file slideshow.hpp
 *  \brief The global slideshow in the background
 *
 * The background of arcollect is a big slideshow object. His content change
 * depending on the context, when you are on a artist page, the background is a
 * random artwork from him and so.
 * 
 * When in slideshow mode, the user directy control the slideshow.
 */
#include "views.hpp"
#include <sqlite3.hpp>
namespace Arcollect {
	namespace gui {
		/** This is the global background slideshow
		 *
		 */
		extern view_slideshow background_slideshow;
		/** Update the background image
		 */
		void update_background(db::artwork_id artid);
		/** Update the background image
		 * \param stmt       A ready SQLite3 statement that yield one column with
		 *                   the artwork id to show.
		 * \param collection If true, pull all images and set a slideshow, else only
		 *                   set one image.
		 *
		 * This function update the background image with the id returned by the SQL
		 * statement passed in option. **This function call sqlite_step().**
		 *
		 * If collection is true, the stmt will be stealed.
		 *
		 * It is used to make the background responsive to the context.
		 */
		void update_background(std::unique_ptr<SQLite3::stmt> &stmt, bool collection);
		/** Update the background image
		 */
		void update_background(bool collection);
	}
}
