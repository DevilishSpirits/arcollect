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
#include "artwork-loader.hpp"
std::mutex Arcollect::db::artwork_loader::lock;
std::vector<std::shared_ptr<Arcollect::db::artwork>> Arcollect::db::artwork_loader::pending_main;
std::vector<std::shared_ptr<Arcollect::db::artwork>> Arcollect::db::artwork_loader::pending_thread;
std::unordered_map<std::shared_ptr<Arcollect::db::artwork>,std::unique_ptr<SDL::Surface>> Arcollect::db::artwork_loader::done;
std::condition_variable Arcollect::db::artwork_loader::condition_variable;
bool Arcollect::db::artwork_loader::stop = false;
std::size_t Arcollect::db::artwork_loader::image_memory_usage = 0;

static void main_thread(void)
{
	using namespace Arcollect::db::artwork_loader;
	while (1) {
		// Find an artwork to load
		std::shared_ptr<Arcollect::db::artwork> artwork;
		{
			std::unique_lock<std::mutex> lock_guard(lock);
			while (!pending_thread.size()) {
				if (Arcollect::db::artwork_loader::stop)
					return;
				condition_variable.wait(lock_guard);
			}
			// Pop artwork
			artwork = pending_thread.back();
			pending_thread.pop_back();
			// Ensure the artwork is not already loaded
			if (done.find(artwork) != done.end())
				continue;
		}
		// Load the artwork
		SDL::Surface *surf = artwork->load_surface();
		// Queue the surface
		{
			std::lock_guard<std::mutex> lock_guard(lock);
			if (done.size() == 0) {
				// No art done, wake-up the main thread ONLY once (hence the if)
				SDL_Event e;
				e.type = SDL_USEREVENT;
				SDL_PushEvent(&e);
			}
			done.emplace(artwork,surf);
		}
	}
}
std::thread Arcollect::db::artwork_loader::thread(main_thread);
