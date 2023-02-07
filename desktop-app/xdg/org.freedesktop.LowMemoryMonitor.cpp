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

DBusHandlerResult Arcollect::dbus::handle_LowMemoryWarning(DBusConnection *conn, DBusMessage *message, void*)
{
	if (dbus_message_is_signal(message,"org.freedesktop.LowMemoryMonitor","LowMemoryWarning")
	||  dbus_message_is_signal(message,"org.freedesktop.portal.MemoryMonitor","LowMemoryWarning")
	) {
		uint8_t value;
		if (dbus_message_get_args(message,NULL,DBUS_TYPE_BYTE,&value,DBUS_TYPE_INVALID)) {
			if (value == 255) return DBUS_HANDLER_RESULT_HANDLED; // FIXME Daemon seem to always throw a 255 after the normal signal ???
			if (value <  50) return DBUS_HANDLER_RESULT_HANDLED;
			//  50 -> Memory is low, should free up unneeded resources so they can be used elsewhere.
			// TODO Perform a quick GC
			if (value < 100) return DBUS_HANDLER_RESULT_HANDLED;
			// 100 -> Should try harder to free up unneeded resources. If does not need to stay running, it is a good time to quit.
			if (!Arcollect::gui::enabled)
				Arcollect::dbus::exit_if_idle();
			// TODO Perform an aggressive garbage collection
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	} else return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
