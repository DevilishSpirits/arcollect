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
#include <config.h>
#include "dbus.hpp"
// TODO Put this in a dedicated XML file
static const char* dbus_introspection =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<!-- Arcollect " ARCOLLECT_VERSION_STR " -->\n"
"<node name=\"" ARCOLLECT_DBUS_PATH_STR "\">\n"
"	<interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"		<method name=\"Introspect\">\n"
"			<arg type=\"s\" name=\"xml_data\" direction=\"out\"/>\n"
"		</method>\n"
"	</interface>\n"
"	<interface name=\"org.freedesktop.DBus.Peer\">\n"
"		<method name=\"Ping\"/>\n"
"		<method name=\"GetMachineId\">\n"
"			<arg type=\"s\" name=\"machine_uuid\" direction=\"out\"/>\n"
"		</method>\n"
"	</interface>\n"
"	<interface name=\"org.gnome.Shell.SearchProvider2\">\n"
"		<method name=\"GetInitialResultSet\">\n"
"			<arg type=\"as\" name=\"terms\" direction=\"in\"/>\n"
"			<arg type=\"as\" name=\"results\" direction=\"out\"/>\n"
"		</method>\n"
"		<method name=\"GetSubsearchResultSet\">\n"
"			<arg type=\"as\" name=\"previous_results\" direction=\"in\"/>\n"
"			<arg type=\"as\" name=\"terms\" direction=\"in\"/>\n"
"			<arg type=\"as\" name=\"results\" direction=\"out\"/>\n"
"		</method>\n"
"		<method name=\"GetResultMetas\">\n"
"			<arg type=\"as\" name=\"identifiers\" direction=\"in\"/>\n"
"			<arg type=\"aa{sv}\" name=\"metas\" direction=\"out\"/>\n"
"		</method>\n"
"		<method name=\"ActivateResult\">\n"
"			<arg type=\"s\" name=\"identifier\" direction=\"in\"/>\n"
"			<arg type=\"as\" name=\"terms\" direction=\"in\"/>\n"
"			<arg type=\"u\" name=\"timestamp\" direction=\"in\"/>\n"
"		</method>\n"
"		<method name=\"LaunchSearch\">\n"
"			<arg type=\"as\" name=\"terms\" direction=\"in\"/>\n"
"			<arg type=\"u\" name=\"timestamp\" direction=\"in\"/>\n"
"		</method>\n"
"	</interface>\n"
"</node>\n"
;

static DBusHandlerResult reply_error(DBus::Connection &conn, DBusMessage *message, const char* err, const char *msg)
{
	DBusMessage *reply = dbus_message_new_error(message,err,msg);
	conn.send(reply);
	dbus_message_unref(reply);
	return DBUS_HANDLER_RESULT_HANDLED;
}
DBusHandlerResult Arcollect::dbus::reply_unknow_method(DBus::Connection &conn, DBusMessage *message)
{
	return reply_error(conn,message,DBUS_ERROR_UNKNOWN_METHOD,"Unknow method.");
}
static DBusHandlerResult reply_unknow_interface(DBus::Connection &conn, DBusMessage *message)
{
	return reply_error(conn,message,DBUS_ERROR_UNKNOWN_INTERFACE,"Unknow interface.");
}

static DBusHandlerResult root_handler(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBus::Connection conn(connection);
	if (dbus_message_has_interface(message,"org.freedesktop.DBus.Introspectable"))
		if (dbus_message_has_member(message,"Introspect")) {
			// Reply with introspection data
			DBusMessage *reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply,DBUS_TYPE_STRING,&dbus_introspection,DBUS_TYPE_INVALID);
			conn.send(reply);
			dbus_message_unref(reply);
		} else return Arcollect::dbus::reply_unknow_method(conn,message);
	else if (dbus_message_has_interface(message,"org.gnome.Shell.SearchProvider2")) {
		return Arcollect::dbus::gnome_shell_search_provider_intf(conn,message);
	} else return reply_unknow_interface(conn,message);
	return DBUS_HANDLER_RESULT_HANDLED;
}
const DBusObjectPathVTable Arcollect::dbus::root_handler_vtable = {NULL,root_handler,NULL,NULL,NULL,NULL};
