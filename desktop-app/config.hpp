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
#pragma once
#include <iostream>
#include <arcollect-paths.hpp>
#include <config.h>
namespace Arcollect {
	namespace config {
		void read_config(void);
		void write_config(void);
		
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
				 */
				inline Param& operator=(const T& new_value) {
					value = new_value;
					write_config();
					return *this;
				}
				const T default_value;
				Param(const T default_value) : value(default_value), default_value(default_value) {};
		};
		/** first_run - If you did the first run for this version.
		 * 
		 * This is an int used to display the first run tutorial.
		 */
		extern Param<int> first_run;
		/** start_fullscreen - Start in fullscreen
		 */
		enum StartWindowMode {
			STARTWINDOW_NORMAL     = 0,
			STARTWINDOW_MAXIMIZED  = 1,
			STARTWINDOW_FULLSCREEN = 2,
		};
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
	}
}
