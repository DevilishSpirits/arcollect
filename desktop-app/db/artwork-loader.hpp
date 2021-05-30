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
#include "artwork.hpp"
#include <arcollect-paths.hpp>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <SDL_image.h>
namespace Arcollect {
	namespace db {
		/** Asynchronous artwork loading utility
		 *
		 * Artwork are usully heavy. Their loading from the disk is offloaded to a
		 * background thread. This is the public interface
		 */
		namespace artwork_loader {
			/** Global mutex
			 *
			 * This mutex protect access to #pending and #done vectors.
			 */
			extern std::mutex lock;
			/** Pending artwork list (main thread side)
			 *
			 * This vector is populated with pending artworks to load and then copied
			 * into #pending_thread.
			 *
			 * It is only used by the main thread.
			 */
			extern std::vector<std::shared_ptr<Arcollect::db::artwork>> pending_main;
			/** Pending artwork list (thread side)
			 *
			 * This vector contain the list of pending artworks to load.
			 * 
			 * It is used by the loading thread, the main thread regulary erase it
			 * with #pending_main content.
			 */
			extern std::vector<std::shared_ptr<Arcollect::db::artwork>> pending_thread;
			/** Loaded artwork surface list
			 *
			 * This vector contain the list of loaded surfaces. The main thread will
			 * then load surfaces into textures.
			 */
			extern std::unordered_map<std::shared_ptr<Arcollect::db::artwork>,std::unique_ptr<SDL::Surface>> done;
			extern bool stop;
			extern std::thread thread;
			extern std::condition_variable condition_variable;
		}
	}
}
