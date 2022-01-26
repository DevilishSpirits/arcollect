/* Arcollect -- An artwork collection manager
 * Copyright (C) 2021 DevilishSpirits (aka D-Spirits or Luc B.)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
		extern view_slideshow &background_slideshow;
		extern view_vgrid     &background_vgrid;
		/** Update the background collection
		 * \param collection Collection
		 */
		void update_background(std::shared_ptr<Arcollect::db::artwork_collection> &new_collection);
		/** Update the background image
		 * \param search Search expression
		 */
		void update_background(const std::string &search);
		/** Get current search filter
		 * \return The last search param of update_background() call.
		 */
		const std::string get_current_search(void);
	}
}
