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
#include "sdl2-hpp/SDL.hpp"
#include <arcollect-db-open.hpp>
#include <arcollect-db-schema.hpp>
#include <arcollect-debug.hpp>
#include "config.hpp"
#include "db/artwork-loader.hpp"
#include "db/db.hpp"
#include "db/filter.hpp"
#include "gui/animation.hpp"
#include "gui/artwork-collections.hpp"
#include "gui/first-run.hpp"
#include "gui/modal.hpp"
#include "gui/rating-selector.hpp"
#include "gui/slideshow.hpp"
#include "gui/views.hpp"
#include "gui/font.hpp"
#include "gui/window-borders.hpp"
#include <iostream>
#include <string>


extern SDL_Window    *window;
SDL_Window    *window;
extern SDL::Renderer *renderer;
SDL::Renderer *renderer;
std::vector<std::reference_wrapper<Arcollect::gui::modal>> Arcollect::gui::modal_stack;

// animation.hpp variables
bool   Arcollect::gui::animation_running;
Uint32 Arcollect::gui::time_now;
Uint32 Arcollect::gui::time_framedelta;

static bool bool_debug_flag(const char* env_var)
{
	const char* value = std::getenv(env_var);
	return (value != NULL) && (env_var[0] != '\0') && (env_var[0] != '0');
}

#define WITH_DEBUG // TODO Made this a project option
int main(int argc, char *argv[])
{
	// Read config
	Arcollect::config::read_config();
	#ifdef WITH_DEBUG
	/** Debug redraws.
	 *
	 * Print frame rate and a moving square from left to right at each frame redraw.
	 */
	bool debug_redraws = Arcollect::debug::is_on("redraws");
	SDL::Rect debug_redraws_rect{0,0,32,32};
	#endif
	// Init SDL
	SDL::Hint::SetRenderScaleQuality(SDL::Hint::RENDER_SCALE_QUALITY_BEST);
	if (SDL::Init(SDL::INIT_VIDEO)) {
		std::cerr << "SDL initialization failed: " << SDL::GetError() << std::endl;
		return 1;
	}
	int window_create_flags = SDL_WINDOW_RESIZABLE;
	switch (Arcollect::config::start_window_mode) {
		case Arcollect::config::STARTWINDOW_NORMAL: {
			window_create_flags |= 0; // Add no flag
		} break;
		case Arcollect::config::STARTWINDOW_MAXIMIZED: {
			window_create_flags |= SDL_WINDOW_MAXIMIZED;
		} break;
		case Arcollect::config::STARTWINDOW_FULLSCREEN: {
			window_create_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		} break;
	}
	if (SDL::CreateWindowAndRenderer(600,400,window_create_flags,window,renderer)) {
		std::cerr << "Failed to create window: " << SDL::GetError() << std::endl;
		return 1;
	}
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	// Load font
	// TODO Use real font management
	TTF_Init();
	// Set custom borders
	Arcollect::gui::window_borders::init(window);
	// Get window size
	SDL::Rect window_rect{0,0};
	renderer->GetOutputSize(window_rect.w,window_rect.h);
	// Load the db
	Arcollect::database = Arcollect::db::open();
	std::unique_ptr<SQLite3::stmt> preload_artworks_stmt;
	if (Arcollect::database->prepare(Arcollect::db::schema::preload_artworks,preload_artworks_stmt) != SQLITE_OK)
		std::cerr << "Failed to prepare preload_artworks SQL stmt: " << Arcollect::database->errmsg() << std::endl;
	// Bootstrap the background
	Arcollect::gui::update_background(true);
	Arcollect::gui::background_slideshow.resize(window_rect);
	Arcollect::gui::modal_stack.push_back(Arcollect::gui::background_slideshow);
	// Show the first run if not done
	if (Arcollect::config::first_run == 0)
		Arcollect::gui::modal_stack.push_back(Arcollect::gui::first_run_modal);
	// Main-loop
	SDL::Event e;
	bool done = false;
	Arcollect::gui::time_now = SDL_GetTicks();
	while (true/*!done*/) {
		// Wait for event
		bool saved_animation_running = Arcollect::gui::animation_running;
		Arcollect::gui::animation_running = false;
		bool has_event = saved_animation_running ? SDL::PollEvent(e) : SDL::WaitEvent(e);
		// Check for emergency SFW shortcut (Ctrl+Maj+X)
		if ((SDL_GetModState()&(KMOD_CTRL|KMOD_SHIFT)) && keyboard_state[SDL_GetScancodeFromKey(SDLK_x)])
			Arcollect::db_filter::set_rating(Arcollect::config::RATING_NONE);
		// Update timing informations
		Uint32 new_ticks = SDL_GetTicks();
		Arcollect::gui::time_framedelta = Arcollect::gui::time_now-new_ticks;
		Arcollect::gui::time_now = new_ticks;
		// Handle event
		while (has_event) {
			if (Arcollect::gui::window_borders::event(e)) {
				// Propagate event to modals
				auto iter = Arcollect::gui::modal_stack.rbegin();
				while (iter->get().event(e))
					++iter;
			}
			if (e.type == SDL_QUIT) {
				done = true;
				break;
			}
			// Process other events
			has_event = SDL::PollEvent(e);
		}
		if (done)
			break;
		// Check for DB updates
		Arcollect::update_data_version();
		// Render frame
		renderer->SetDrawColor(0,0,0,0);
		renderer->Clear();
		for (auto& iter: Arcollect::gui::modal_stack) {
			// Draw a backdrop
			renderer->SetDrawBlendMode(SDL::BLENDMODE_BLEND);
			renderer->SetDrawColor(0,0,0,128);
			renderer->FillRect();
			// Render
			iter.get().render();
		}
		Arcollect::gui::window_borders::render();
		#ifdef WITH_DEBUG
		if (!debug_redraws)
		#endif
		renderer->Present();
		// Erase artwork_loader pending list and load artworks into texture
		{
			std::lock_guard<std::mutex> lock_guard(Arcollect::db::artwork_loader::lock);
			// Load artworks
			if (Arcollect::db::artwork_loader::done.size()) {
				// Generate a redraw
				Arcollect::gui::animation_running = true;
				// Load arts
				for (auto &art: Arcollect::db::artwork_loader::done) {
					std::unique_ptr<SDL::Texture> text(SDL::Texture::CreateFromSurface(renderer,art.second.get()));
					art.first->texture_loaded(text);
				}
				Arcollect::db::artwork_loader::done.clear();
			}
			// Update pending list
			if (Arcollect::db::artwork_loader::pending_main.size())
				Arcollect::db::artwork_loader::pending_thread = std::move(Arcollect::db::artwork_loader::pending_main);
		}
		// Query artworks to preload
		if (preload_artworks_stmt) {
			preload_artworks_stmt->reset();
			while (preload_artworks_stmt->step() == SQLITE_ROW)
				Arcollect::db::artwork::query(preload_artworks_stmt->column_int64(0))->queue_for_load();
		}
		#ifdef WITH_DEBUG
		// Redraws debugging
		if (debug_redraws) {
			static Uint32 last_frame_time_now = 0;
			debug_redraws_rect.x++;
			debug_redraws_rect.x %= 500;
			renderer->SetDrawColor(255,255,255,255);
			renderer->FillRect(debug_redraws_rect);
			renderer->SetDrawColor(0,0,0,255);
			renderer->DrawRect(debug_redraws_rect);
			Uint32 render_time = SDL_GetTicks() - new_ticks;
			Uint32 events_time = new_ticks - last_frame_time_now;
			std::cerr << "Render: " << render_time << "ms/" << 1000.f/render_time << " FPS"
			          << "\tEvents:" << events_time << "ms" 
			          << "\tTotal:" << render_time + events_time << "ms/" << 1000.f/(render_time + events_time) << " FPS\t("
			          <<  "animation_running:" << (saved_animation_running ? "y" : "n")
			          << ")." << std::endl;
			last_frame_time_now = new_ticks;
			renderer->Present();
		}
		#endif
	}
	// Cleanups
	// Erase artwork_loader pending list
	{
		std::lock_guard<std::mutex> lock_guard(Arcollect::db::artwork_loader::lock);
		Arcollect::db::artwork_loader::pending_thread.clear();
	}
	Arcollect::db::artwork_loader::stop = true;
	Arcollect::db::artwork_loader::condition_variable.notify_one();
	Arcollect::db::artwork_loader::thread.join();
	return 0;
}
