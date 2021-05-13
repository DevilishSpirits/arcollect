#include "artwork-loader.hpp"
std::mutex Arcollect::db::artwork_loader::lock;
std::vector<std::shared_ptr<Arcollect::db::artwork>> Arcollect::db::artwork_loader::pending_main;
std::vector<std::shared_ptr<Arcollect::db::artwork>> Arcollect::db::artwork_loader::pending_thread;
std::unordered_map<std::shared_ptr<Arcollect::db::artwork>,std::unique_ptr<SDL::Surface>> Arcollect::db::artwork_loader::done;
std::condition_variable Arcollect::db::artwork_loader::condition_variable;
bool Arcollect::db::artwork_loader::stop = false;

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
			done.emplace(artwork,surf);
		}
	}
}
std::thread Arcollect::db::artwork_loader::thread(main_thread);
