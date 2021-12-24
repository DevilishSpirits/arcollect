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
/** \file config.hpp
 *  \brief Arcollect user preferences utilities
 *
 * This header define the Arcollect::config namespace that contain user
 * preferences of Arcollect (`arcollect.ini`). It is separate from the database.
 */
#pragma once
#include <iostream>
#include <arcollect-paths.hpp>
#include <config.h>
#include <cstddef>
namespace Arcollect {
	/** `arcollect.ini` file configuration namespace
	 *
	 * Arcollect store it's configuration within `$XDG_CONFIG_HOME/arcollect.ini`
	 */
	namespace config {
		/** Read arcollect.ini
		 *
		 * This function is called at startup
		 */
		void read_config(void);
		/** Write arcollect.ini
		 *
		 * This function is called when a #Param is set
		 */
		void write_config(void);
		
		/** High-level encapsulation of a user preference setting
		 *
		 * It come with a default value and setting it automatically update the
		 * configuration file.
		 */
		template<typename T>
		class Param {
			private:
				friend void read_config(void);
				T value;
			public:
				/** Read the param
				 */
				inline operator const T&(void) const {
					return value;
				}
				/** Write the param
				 *
				 * This operator overload automatically call write_config(). The change
				 * is persistent.
				 */
				inline Param& operator=(const T& new_value) {
					value = new_value;
					write_config();
					return *this;
				}
				const T default_value;
				Param(const T default_value) : value(default_value), default_value(default_value) {};
		};
		enum StartWindowMode {
			STARTWINDOW_NORMAL     = 0,
			STARTWINDOW_MAXIMIZED  = 1,
			STARTWINDOW_FULLSCREEN = 2,
		};
		/** start_window_mode - The window mode to use at startup
		 */
		extern Param<int> start_window_mode;
		
		enum Rating: int {
			RATING_NONE   = 0,
			RATING_MATURE = 16,
			RATING_ADULT  = 18,
		};
		/** current_rating - Current artwork rating option
		 *
		 * This is a global filter on displayed artworks
		 */
		extern Param<int> current_rating;
		
		/** image_memory_limit - The maximum amount of memory used by images in MiB
		 *
		 * This is a kind of VRAM limit. Arcollect will unload artworks when the
		 * size of loaded pixels exceed this value.
		 *
		 * The consumed memory is estimated by computing the size needed to store
		 * artworks in a uncompressed form.
		 */
		extern Param<int> image_memory_limit;
		
		/** littlecms_intent - Color management rendering intent
		 */
		extern Param<int> littlecms_intent;
		
		/** littlecms_flags - Color management flags
		 *
		 * This is the flags passed to LittleCMS cmsCreateTransform() call.
		 */
		extern Param<int> littlecms_flags;
		
		/** writing_font_size - Font height of textual artworks in pixels
		 *
		 * This is the text height in pixels used to render text artworks.
		 */
		extern Param<int> writing_font_size;
		
		/** rows_per_screen - Number of rows per screen
		 *
		 * This adjust the height of rows in the grid view to display the given
		 * number of rows at full screen.
		 */
		extern Param<int> rows_per_screen;
	}
}
