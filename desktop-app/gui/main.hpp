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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
/** \file desktop-app/gui/main.hpp
 *  \brief Splitted main-loop
 *
 * The "real" Arcollect main-loop is splitted in multiple functions to allow
 * platform dependant loops like the D-Bus one and ease testing.
 *
 * To make things simple, here is the generic main `desktop-app/main.cpp` :
 * \include{lineno} desktop-app/main.cpp
 */
#pragma once
namespace Arcollect {
	namespace gui {
		/** Init the GUI part
		 * \return Zero on success
		 *
		 * This must be called once before the main(). It init graphic resources.
		 */
		int init(void);
		/** GUI show
		 */
		void start(int argc, char** argv);
		/** Wether the GUI is on
		 */
		extern bool enabled;
		/** Main-loop of the GUI part
		 * \return false if the app should quit
		 *
		 * This function is blocking.
		 */
		bool main(void);
		/** Wake-up the main-loop
		 *
		 * This function push a dummy SDL event that trigger a new main-loop run.
		 */
		void wakeup_main(void);
		/** GUI hide
		 */
		void stop(void);
	}
}
