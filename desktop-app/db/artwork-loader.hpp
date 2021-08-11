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
#include <memory>
#include <unordered_map>
#include <vector>
#include <OpenImageIO/imageio.h>
namespace Arcollect {
	namespace db {
		/** Asynchronous artwork loading utility
		 *
		 * Artwork are usully heavy. Their loading from the disk is offloaded to a
		 * background thread. The public part is used like a namespace.
		 */
		class artwork_loader: private std::thread {
			private:
				static std::vector<std::unique_ptr<artwork_loader>> threads;
				/** Thread stop flag
				 *
				 * If true, the thread exit.
				 */
				volatile bool stop = false;
				static void thread_func(volatile bool &stop);
				artwork_loader(void) : std::thread(thread_func,std::ref(stop)) {}
			public:
				~artwork_loader(void);
				/** Global mutex
				 *
				 * This mutex protect access to #pending and #done vectors.
				 */
				static std::mutex lock;
				/** Pending artwork list (main thread side)
				 *
				 * This vector is populated with pending artworks to load and then
				 * appended into #pending_thread.
				 *
				 * It is only used by the main thread.
				 */
				static std::vector<std::shared_ptr<Arcollect::db::artwork>> pending_main;
				/** Pending artwork list (thread side)
				 *
				 * This vector contain the list of artworks to load in first place.
				 * 
				 * It is used by the loading thread, the main thread regulary move the
				 * #pending_main content into.
				 */
				static std::vector<std::shared_ptr<Arcollect::db::artwork>> pending_thread_first;
				/** Pending artwork list (thread side)
				 *
				 * This vector contain the list of pending artworks to load.
				 * 
				 * The loading thread load these artworks if #pending_thread_first is
				 * empty. The main thread regulary append the #pending_main content
				 * into this list (as opposed to move into in for #pending_thread_first).
				 */
				static std::vector<std::shared_ptr<Arcollect::db::artwork>> pending_thread_second;
				/** Loaded artwork surface list
				 *
				 * This vector contain the list of loaded surfaces. The main thread will
				 * then load surfaces into textures.
				 */
				static std::unordered_map<std::shared_ptr<Arcollect::db::artwork>,std::unique_ptr<SDL::Surface>> done;
				static std::condition_variable condition_variable;
				
				/** Estimation of VRAM usage of artworks in bytes
				 *
				 * This value is used to enforce Arcollect::config::image_memory_limit.
				 * It's updated by artworks.
				 */
				static std::size_t image_memory_usage;
				
				/** Start the background thread
				 *
				 * \warning Must not be called if the thread has been start() already
				 *          and has not been shutdown().
				 */
				static void start(void);
				/** Shutdown the background thread
				 */
				static void shutdown(void);
		};
	}
}
