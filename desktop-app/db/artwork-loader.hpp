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
#if WITH_MINGW_STD_THREADS
#include <mingw.condition_variable.h>
#include <mingw.mutex.h>
#include <mingw.thread.h>
#else
#include <condition_variable>
#include <mutex>
#include <thread>
#endif
#include <memory>
#include <unordered_map>
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
				static std::unique_ptr<artwork_loader> thread;
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
				 * This vector is populated with pending artworks to load and then copied
				 * into #pending_thread.
				 *
				 * It is only used by the main thread.
				 */
				static std::vector<std::shared_ptr<Arcollect::db::artwork>> pending_main;
				/** Pending artwork list (thread side)
				 *
				 * This vector contain the list of pending artworks to load.
				 * 
				 * It is used by the loading thread, the main thread regulary erase it
				 * with #pending_main content.
				 */
				static std::vector<std::shared_ptr<Arcollect::db::artwork>> pending_thread;
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
