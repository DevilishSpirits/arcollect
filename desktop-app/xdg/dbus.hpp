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
#include <sqlite3.hpp>
#include "dbus-helper.hpp"
#include <memory>
namespace Arcollect {
	namespace dbus {
		extern const DBusObjectPathVTable root_handler_vtable;
		
		DBusHandlerResult reply_unknow_method(DBus::Connection &conn, DBusMessage *message);
		
		DBusHandlerResult freedesktop_application_intf(DBus::Connection &conn, DBusMessage *message);
		DBusHandlerResult gnome_shell_search_provider_intf(DBus::Connection &conn, DBusMessage *message);
		extern std::unique_ptr<SQLite3::stmt> gnome_shell_search_provider_result_metas_stmt;
		
		/** Ask the D-Bus main-loop to exit if idle
		 */
		void exit_if_idle(void);
		DBusHandlerResult handle_LowMemoryWarning(DBusConnection *conn, DBusMessage *message, void*);
	}
}
