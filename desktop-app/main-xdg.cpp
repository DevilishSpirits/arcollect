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
/** \file main.cpp
 *  \brief Implementation of main() with XDG integration
 *
 * The XDG integration allow Arcollect to run in background listening for D-Bus
 * messages with some desktop-specific integration.
 */
#include <arcollect-db-open.hpp>
#include <config.h>
#include "config.hpp"
#include "db/db.hpp"
#include "xdg/dbus.hpp"
#include "gui/main.hpp"
#include "sdl2-hpp/SDL.hpp"
#include <thread>

static volatile bool dbus_continue = true;
static void dbus_thread_func(DBus::Connection conn)
{
	while (dbus_continue && conn.read_write(100))
		if (dbus_connection_get_dispatch_status(conn) == DBUS_DISPATCH_DATA_REMAINS) {
			// Wake-up main loop
			SDL_Event e;
			e.type = SDL_USEREVENT;
			SDL_PushEvent(&e);
		}
}

int main(int argc, char *argv[])
{
	// WARNING! Init order is important.
	// Read config
	Arcollect::config::read_config();
	// Load the db
	Arcollect::database = Arcollect::db::open();
	
	Arcollect::database->prepare("SELECT art_title, trim(art_desc) FROM artworks WHERE art_artid = ?;",Arcollect::dbus::gnome_shell_search_provider_result_metas_stmt); // TODO Error checkings
	// Init the GUI
	if (Arcollect::gui::init())
		return 1;
	// Init D-Bus
	dbus_threads_init_default();
	DBus::Connection conn(DBus::BUS_SESSION);
	conn.set_exit_on_disconnect(false);
	conn.bus_request_name(ARCOLLECT_DBUS_NAME_STR,DBUS_NAME_FLAG_ALLOW_REPLACEMENT|DBUS_NAME_FLAG_REPLACE_EXISTING);
	dbus_connection_register_fallback(conn,"/",&Arcollect::dbus::root_handler_vtable,NULL);
	// Start dbus thread
	std::thread dbus_thread(dbus_thread_func,conn);
	dbus_thread.detach();
	// Run GUI main-loop
	Arcollect::gui::start();
	while (Arcollect::gui::main())
		conn.dispatch();
	dbus_continue = false;
	Arcollect::gui::stop();
	return 0;
}
