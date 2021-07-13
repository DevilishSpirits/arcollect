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
#include "../db/artwork.hpp"
#include "../db/db.hpp"
#include "../db/search.hpp"

std::unique_ptr<SQLite3::stmt> Arcollect::dbus::gnome_shell_search_provider_result_metas_stmt;

static DBusHandlerResult GetResultSet(DBus::Connection &conn, DBusMessage *message, DBus::Message::iterator terms)
{
	// Rebuild search_string that we'll retokenize again
	std::string search_string;
	for (auto term: terms) {
		search_string += " " + std::string(term.get_basic<const char*>());
	}
	
	// Prepare reply
	DBusMessage *reply = dbus_message_new_method_return(message);
	DBus::append_iterator append_iter(reply);
	DBus::append_iterator results;
	append_iter.open_container('a',"s",results);
	
	// Build statement
	std::unique_ptr<SQLite3::stmt> stmt;
	if (!Arcollect::db::search::build_stmt(search_string.c_str(),stmt))
		while (stmt->step() == SQLITE_ROW) {
			// Append artwork id
			results << std::to_string(stmt->column_int64(0)).c_str();
		}
	append_iter.close_container(results);
	
	// Send message
	conn.send(reply);
	dbus_message_unref(reply);
	return DBUS_HANDLER_RESULT_HANDLED;
}

static void add_dbus_dict_sv(DBus::append_iterator& result, const char* key, const char* value)
{
	DBus::append_iterator dict;
	DBus::append_iterator variant;
	result.open_container('e',NULL,dict);
		dict << key;
		dict.open_container('v',"s",variant);
			variant << value;
		dict.close_container(variant);
	result.close_container(dict);
}
static DBusHandlerResult GetResultMetas(DBus::Connection &conn, DBusMessage *message)
{
	// Prepare reply
	DBusMessage *reply = dbus_message_new_method_return(message);
	DBus::append_iterator append_iter(reply);
	DBus::append_iterator results;
	append_iter.open_container('a',"a{sv}",results);
	// Loop
	for (auto iter: DBus::Message::iterator(message)) {
		const char* id = iter.get_basic<const char*>();
		std::shared_ptr<Arcollect::db::artwork> artwork = Arcollect::db::artwork::query(std::atoi(id));
		if (artwork) {
			DBus::append_iterator result;
			results.open_container('a',"{sv}",result);
			add_dbus_dict_sv(result,"id",id);
			add_dbus_dict_sv(result,"name",artwork->title().c_str());
			add_dbus_dict_sv(result,"description",artwork->desc().c_str());
			add_dbus_dict_sv(result,"gicon",(Arcollect::path::artwork_pool / std::string(id)).string().c_str());
			results.close_container(result);
		}
	}
	append_iter.close_container(results);
	// Send message
	conn.send(reply);
	dbus_message_unref(reply);
	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult Arcollect::dbus::gnome_shell_search_provider_intf(DBus::Connection &conn, DBusMessage *message)
{
	if (dbus_message_has_member(message,"GetInitialResultSet")) {
		return GetResultSet(conn,message,  DBus::Message::iterator(message));
	} else if (dbus_message_has_member(message,"GetSubsearchResultSet")) {
		return GetResultSet(conn,message,++DBus::Message::iterator(message));
	} else if (dbus_message_has_member(message,"GetResultMetas")) {
		return GetResultMetas(conn,message);
	} else return Arcollect::dbus::reply_unknow_method(conn,message);
}

