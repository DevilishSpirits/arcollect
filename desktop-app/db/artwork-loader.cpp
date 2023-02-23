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
#include "../config.hpp"
#include "artwork-loader.hpp"
std::mutex Arcollect::db::artwork_loader::lock;
std::vector<std::shared_ptr<Arcollect::db::download>> Arcollect::db::artwork_loader::pending_main;
std::vector<std::shared_ptr<Arcollect::db::download>> Arcollect::db::artwork_loader::pending_thread_first;
std::vector<std::shared_ptr<Arcollect::db::download>> Arcollect::db::artwork_loader::pending_thread_second;
std::unordered_set<std::shared_ptr<Arcollect::db::download>> Arcollect::db::artwork_loader::done;
std::condition_variable Arcollect::db::artwork_loader::condition_variable;
std::size_t Arcollect::db::artwork_loader::image_memory_usage = 0;
std::vector<std::unique_ptr<Arcollect::db::artwork_loader>> Arcollect::db::artwork_loader::threads;

void Arcollect::db::artwork_loader::thread_func(volatile bool &stop)
{
	while (!stop) {
		// Find an artwork to load
		std::shared_ptr<Arcollect::db::download> artwork;
		{
			std::unique_lock<std::mutex> lock_guard(lock);
			while (pending_thread_first.empty() && pending_thread_second.empty()) {
				if (stop)
					return;
				condition_variable.wait(lock_guard);
			}
			// Pop artwork
			auto &pending_thread = pending_thread_first.size() ? pending_thread_first : pending_thread_second;
			artwork = pending_thread.back();
			pending_thread.pop_back();
			// Skip if it has already been loaded
			if (artwork->load_state != artwork->LOAD_PENDING_STAGE1)
				continue;
			// Lock the artwork (it might be in both pending_thread_first and pending_thread_second)
			artwork->load_state = artwork->LOADING_STAGE1;
		}
		// Check if the artwork is worth to load
		if (artwork->keep_loaded())
			artwork->load_stage_one();
		else artwork->load_state = artwork->UNLOADED;
		// Queue the artwork for load
		if (artwork->load_state == artwork->LOAD_PENDING_STAGE2) {
			std::lock_guard<std::mutex> lock_guard(lock);
			if (done.size() == 0) {
				// No art done, wake-up the main thread ONLY once (hence the if)
				SDL_Event e;
				e.type = SDL_USEREVENT;
				SDL_PushEvent(&e);
			}
			done.emplace(artwork);
			artwork->load_state = artwork->LOAD_PENDING_STAGE2;
		}
	}
}

void Arcollect::db::artwork_loader::start(void)
{
	Arcollect::db::artwork_loader::threads.clear();
	// Use all cores minus 1
	Arcollect::db::artwork_loader::threads.emplace_back(new Arcollect::db::artwork_loader());
	for (unsigned int i = 2; i < std::thread::hardware_concurrency(); i++)
		Arcollect::db::artwork_loader::threads.emplace_back(new Arcollect::db::artwork_loader());
}
void Arcollect::db::artwork_loader::shutdown(void)
{
	for (auto& thread: Arcollect::db::artwork_loader::threads)
		thread->stop = true;
	condition_variable.notify_all();
}

Arcollect::db::artwork_loader::~artwork_loader(void)
{
	stop = true;
	condition_variable.notify_all();
	join();
}
