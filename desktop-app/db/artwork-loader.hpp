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
#include "download.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <memory>
#include <unordered_set>
#include <vector>
namespace Arcollect {
	namespace db {
		/** Asynchronous artwork loading system (read this before hacking on it!)
		 *
		 * It's one of the most complex parts of Arcollect that is partly entangled
		 * with Arcollect::db::artwork and Arcollect::gui::main().
		 *
		 * This part is the magic behind Arcollect::db::artwork::queue_for_load().
		 *
		 * It's goal is to "make artwork loads fast". Multithreading help to exploit
		 * full system resources but there is an artwork loading ordering strategy
		 * that truly give this feeling of artworks that load fast.
		 * We must also limit the number of simultaneously loaded artworks.
		 *
		 * To implement our loading strategy, we use multiple queues :
		 * * Arcollect::db::artwork_loader::pending_main that collect which artworks
		 *   which artworks was missing to render the previous frame.
		 * * Arcollect::db::artwork_loader::pending_thread_first that is the
		 *   priority queue. It's replaced with the content of
		 *   Arcollect::db::artwork_loader::pending_main at each frame.
		 * * Arcollect::db::artwork_loader::pending_thread_second is a persistent
		 *   queue of which artworks was requested.
		 * * Arcollect::db::artwork_loader::done contain artwork SDL::Surface that
		 *   Arcollect::gui::main() will transform into SDL::Texture.
		 *
		 * Arcollect::db::artwork_loader::pending_thread_second allows Arcollect to
		 * load recently requested artworks in background even if they wasn't
		 * rendered in the last frame (it was the old technic that was slightly
		 * improved with #1652459). Artworks in this queue have a per artwork
		 * timeout that is reset each time the artwork is requested.
		 *
		 * To implement this, there is Arcollect::db::artwork::LoadState that list
		 * all possibles states of loading, read it. It is set by multiple functions
		 * to track load progress and avoid things like loading multiple times the
		 * artwork (an artwork can be in all queues and handled by multiple threads
		 * simultaneously!).
		 *
		 * The first state change is made by the first call to
		 * Arcollect::db::artwork::queue_for_load() that queue the artwork into
		 * Arcollect::db::artwork_loader::pending_main, then
		 * Arcollect::gui::main() move things into
		 * Arcollect::db::artwork_loader::pending_thread_first and 
		 * Arcollect::db::artwork_loader::pending_thread_second (the later only if
		 * the artworks is no).
		 * A thread will pick up the artwork and mark it as being processed so other
		 * workers will skip it. Once loaded and color managed, the #SDL::Surface is
		 * pushed into Arcollect::db::artwork_loader::done and
		 * Arcollect::gui::main() load it into an SDL::Texture (this must be done
		 * in the main thread) and call Arcollect::db::artwork::texture_loaded().
		 * The artwork is now loaded.
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
				static std::vector<std::shared_ptr<Arcollect::db::download>> pending_main;
				/** Pending artwork list (thread side)
				 *
				 * This vector contain the list of artworks to load in first place.
				 * 
				 * It is used by the loading thread, the main thread regulary move the
				 * #pending_main content into.
				 */
				static std::vector<std::shared_ptr<Arcollect::db::download>> pending_thread_first;
				/** Pending artwork list (thread side)
				 *
				 * This vector contain the list of pending artworks to load.
				 * 
				 * The loading thread load these artworks if #pending_thread_first is
				 * empty. The main thread regulary append the #pending_main content
				 * into this list (as opposed to move into in for #pending_thread_first).
				 */
				static std::vector<std::shared_ptr<Arcollect::db::download>> pending_thread_second;
				/** Loaded artwork surface list
				 *
				 * This vector contain the list of loaded surfaces. The main thread will
				 * then load surfaces into textures.
				 */
				static std::unordered_set<std::shared_ptr<Arcollect::db::download>> done;
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
				
				/** Shutdown and wait for background threads terminations
				 */
				static void shutdown_sync(void) {
					Arcollect::db::artwork_loader::threads.clear();
				}
		};
	}
}
