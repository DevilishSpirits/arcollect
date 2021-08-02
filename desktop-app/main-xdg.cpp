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
 *
 * \note It assume a UNIX-like system.
 */
#include <arcollect-db-open.hpp>
#include <config.h>
#include "config.hpp"
#include "db/db.hpp"
#include "xdg/dbus.hpp"
#include "gui/main.hpp"
#include "sdl2-hpp/SDL.hpp"
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <unordered_map>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

static volatile bool dbus_continue = true;
static Uint32 last_dbus_activity = SDL_GetTicks();

// WARNING! Each sigio_watch_list has a matching sigio_watch_pollfd at same index
static std::vector<DBusWatch*>    sigio_watch_list;
static std::vector<struct pollfd> sigio_watch_pollfd;
static void sigio_handler(int sig)
{
	Arcollect::gui::wakeup_main();
}
static void dbus_wakeup_main(void*)
{
	Arcollect::gui::wakeup_main();
}
static dbus_bool_t dbus_add_watch(DBusWatch *watch, void *data)
{
	// Push watch and a pollfd
	sigio_watch_list.push_back(watch);
	struct pollfd watch_pollfd;
	watch_pollfd.fd = dbus_watch_get_unix_fd(watch);
	auto watch_flags = dbus_watch_get_flags(watch);
	watch_pollfd.events = 0;
	if (dbus_watch_get_enabled(watch)) {
		if (watch_flags & DBUS_WATCH_READABLE)
			watch_pollfd.events |= POLLIN;
		if (watch_flags & DBUS_WATCH_WRITABLE)
			watch_pollfd.events |= POLLOUT;
	}
	sigio_watch_pollfd.push_back(watch_pollfd);
	// Configure the file descriptor
	if (fcntl(watch_pollfd.fd,F_SETOWN,getpid()) == -1)
		perror("In dbus_add_watch(), fcntl(F_SETOWN) failed");
	int fdflags = fcntl(watch_pollfd.fd,F_GETFL);
	if (fdflags == -1) {
		perror("In dbus_add_watch(), fcntl(F_GETFL) failed");
		return true;
	}
	if (fcntl(watch_pollfd.fd,F_SETFL,fdflags|O_ASYNC) == -1) {
		perror("In dbus_add_watch(), fcntl(F_SETFL) with O_ASYNC failed");
		return true;
	}
	return true;
}
static void dbus_watch_toggled(DBusWatch *watch, void *data)
{
	// Find the index of our watch in a dumb way
	decltype(sigio_watch_pollfd)::size_type i;
	for (i = 0; i < sigio_watch_pollfd.size(); i++)
		if (sigio_watch_list[i] == watch)
			break;
	// Update watch flags
	if (i < sigio_watch_list.size()) {
		auto &watch_pollfd = sigio_watch_pollfd[i];
		auto watch_flags = dbus_watch_get_flags(watch);
		watch_pollfd.events = 0;
		if (dbus_watch_get_enabled(watch)) {
			if (watch_flags & DBUS_WATCH_READABLE)
				watch_pollfd.events |= POLLIN;
			if (watch_flags & DBUS_WATCH_WRITABLE)
				watch_pollfd.events |= POLLOUT;
		}
	} else std::cerr << "In dbus_watch_toggled(watch=" << watch << "), the watch was not found." << std::endl;
}
static void dbus_remove_watch(DBusWatch *watch, void *data)
{
	// Find the index of our watch in a dumb way
	decltype(sigio_watch_pollfd)::size_type i;
	for (i = 0; i < sigio_watch_pollfd.size(); i++)
		if (sigio_watch_list[i] == watch)
			break;
	// Simply erase
	if (i < sigio_watch_list.size()) {
		sigio_watch_list.erase(sigio_watch_list.begin()+i);
	} else std::cerr << "In dbus_remove_watch(watch=" << watch << "), the watch was not found." << std::endl;
}

static std::unordered_map<DBusTimeout*,Uint32> timeout_watch_list;
static dbus_bool_t dbus_add_timeout(DBusTimeout *timeout, void *data)
{
	// Add timeout
	timeout_watch_list.emplace(timeout,SDL_GetTicks());
	return true;
}
static void dbus_remove_timeout(DBusTimeout *timeout, void *data)
{
	auto iter = timeout_watch_list.find(timeout);
	if (iter == timeout_watch_list.end())
		std::cerr << "In dbus_remove_timeout(timeout=" << timeout << "), the timeout was not found." << std::endl;
	else timeout_watch_list.erase(iter);
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
	// Install SIGIO handler
	struct sigaction sigio_action;
	sigio_action.sa_handler = NULL;
	sigfillset(&sigio_action.sa_mask);
	sigio_action.sa_flags = 0;
	sigio_action.sa_handler = sigio_handler;
	sigaction(SIGIO,&sigio_action,NULL);
	// Init D-Bus
	dbus_threads_init_default();
	DBus::Connection conn(DBus::BUS_SESSION);
	dbus_connection_set_watch_functions(conn,dbus_add_watch,dbus_remove_watch,dbus_watch_toggled,NULL,NULL);
	dbus_connection_set_timeout_functions(conn,dbus_add_timeout,dbus_remove_timeout,NULL,NULL,NULL);
	dbus_connection_set_wakeup_main_function(conn,dbus_wakeup_main,NULL,NULL);
	conn.set_exit_on_disconnect(false);
	conn.bus_request_name(ARCOLLECT_DBUS_NAME_STR,DBUS_NAME_FLAG_ALLOW_REPLACEMENT|DBUS_NAME_FLAG_REPLACE_EXISTING);
	dbus_connection_register_fallback(conn,"/",&Arcollect::dbus::root_handler_vtable,NULL);
	// Run GUI main-loop
	if ((argc < 2)|| std::strcmp(argv[1],"--dbus-service"))
		Arcollect::gui::start(argc,argv);
	while ((dbus_continue && ((SDL_GetTicks()-last_dbus_activity) < 10000)) || Arcollect::gui::enabled) {
		// Process timeouts
		Uint32 timeout_timestamp = SDL_GetTicks();
		int next_timeout = std::numeric_limits<int>::max();
		for (auto &timeout: timeout_watch_list)
			if (dbus_timeout_get_enabled(timeout.first)) {
				auto interval = dbus_timeout_get_interval(timeout.first);
				int remaining = timeout_timestamp-timeout.second;
				if (remaining >= interval) {
					dbus_timeout_handle(timeout.first);
					timeout.second = timeout_timestamp;
					remaining = interval;
				}
				next_timeout = std::min(next_timeout,remaining);
			}
		// Run GUI
		if (Arcollect::gui::enabled) {
			if (!Arcollect::gui::main())
				Arcollect::gui::stop();
		}
		// Poll D-Bus file descriptors
		if (Arcollect::gui::enabled)
			next_timeout = 0;
		else if (next_timeout == std::numeric_limits<int>::max())
			next_timeout = -1;
		
		if (poll(sigio_watch_pollfd.data(),sigio_watch_pollfd.size(),next_timeout) >= 0) {
			for (decltype(sigio_watch_pollfd)::size_type i = 0; i < sigio_watch_pollfd.size(); i++)
				if (dbus_watch_get_enabled(sigio_watch_list[i])) {
					struct pollfd &watch_pollfd = sigio_watch_pollfd[i];
					int flags = 0;
					if (watch_pollfd.revents & POLLIN)
						flags |= DBUS_WATCH_READABLE;
					if (watch_pollfd.revents & POLLOUT)
						flags |= DBUS_WATCH_WRITABLE;
					if (watch_pollfd.revents & POLLERR)
						flags |= DBUS_WATCH_ERROR;
					if (watch_pollfd.revents & POLLHUP)
						flags |= DBUS_WATCH_HANGUP;
					if (flags)
						dbus_watch_handle(sigio_watch_list[i],flags);
				} else sigio_watch_pollfd[i].events = 0; // Disable the watch
		} else perror("poll() on D-Bus files failed");
		while (dbus_connection_get_dispatch_status(conn) == DBUS_DISPATCH_DATA_REMAINS) {
			conn.dispatch();
			last_dbus_activity = SDL_GetTicks();
		}
	}
	dbus_continue = false;
	return 0;
}
