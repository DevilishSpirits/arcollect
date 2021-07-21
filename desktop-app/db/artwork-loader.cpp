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
#include <arcollect-debug.hpp>
#include "artwork-loader.hpp"
std::mutex Arcollect::db::artwork_loader::lock;
std::vector<std::shared_ptr<Arcollect::db::artwork>> Arcollect::db::artwork_loader::pending_main;
std::vector<std::shared_ptr<Arcollect::db::artwork>> Arcollect::db::artwork_loader::pending_thread;
std::unordered_map<std::shared_ptr<Arcollect::db::artwork>,std::unique_ptr<SDL::Surface>> Arcollect::db::artwork_loader::done;
std::condition_variable Arcollect::db::artwork_loader::condition_variable;
std::size_t Arcollect::db::artwork_loader::image_memory_usage = 0;
std::unique_ptr<Arcollect::db::artwork_loader> Arcollect::db::artwork_loader::thread;

void Arcollect::db::artwork_loader::thread_func(volatile bool &stop)
{
	const bool debug = Arcollect::debug::is_on("artwork-loader");
	while (1) {
		// Find an artwork to load
		std::shared_ptr<Arcollect::db::artwork> artwork;
		{
			std::unique_lock<std::mutex> lock_guard(lock);
			while (!pending_thread.size()) {
				if (stop)
					return;
				condition_variable.wait(lock_guard);
			}
			// Pop artwork
			artwork = pending_thread.back();
			pending_thread.pop_back();
			// Ensure the artwork is not already loaded
			if ((done.find(artwork) != done.end()) || artwork->texture_is_loaded())
				continue;
		}
		// Load the artwork
		SDL::Surface *surf = artwork->load_surface();
		// Queue the surface
		if (surf) {
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

void Arcollect::db::artwork_loader::start(void)
{
	Arcollect::db::artwork_loader::thread.reset(new Arcollect::db::artwork_loader());
}
void Arcollect::db::artwork_loader::shutdown(void)
{
	Arcollect::db::artwork_loader::thread.reset();
}

Arcollect::db::artwork_loader::~artwork_loader(void)
{
	stop = true;
	condition_variable.notify_one();
	join();
}
