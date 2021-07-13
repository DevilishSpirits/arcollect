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
#include "dbus.hpp"
#include "../gui/main.hpp"

DBusHandlerResult Arcollect::dbus::freedesktop_application_intf(DBus::Connection &conn, DBusMessage *message)
{
	if (dbus_message_has_member(message,"Activate")) {
		Arcollect::gui::start(1,NULL);
		return DBUS_HANDLER_RESULT_HANDLED;
	} else return Arcollect::dbus::reply_unknow_method(conn,message);
}

