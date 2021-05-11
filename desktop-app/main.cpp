#include "sdl2-hpp/SDL.hpp"
#include <arcollect-db-open.hpp>
#include "config.hpp"
#include "db/db.hpp"
#include "gui/artwork-collections.hpp"
#include "gui/first-run.hpp"
#include "gui/modal.hpp"
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

Uint32 time_now;
Uint32 time_framedelta;

int main(void)
{
	// Read config
	Arcollect::config::read_config();
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
	// Set custom borders
	Arcollect::gui::window_borders::init(window);
	// Get window size
	SDL::Rect window_rect{0,0};
	renderer->GetOutputSize(window_rect.w,window_rect.h);
	// Load font
	// TODO Use real font management
	TTF_Init();
	// Load the db
	Arcollect::database = Arcollect::db::open();
	// Bootstrap the background
	Arcollect::gui::update_background(true);
	Arcollect::gui::background_slideshow.resize(window_rect);
	Arcollect::gui::modal_stack.push_back(Arcollect::gui::background_slideshow);
	// Main-loop
	SDL::Event e;
	bool not_done = true;
	time_now = SDL_GetTicks();
	while (not_done) {
		// Handle event
		if (SDL::WaitEvent(e)) {
			if (e.type == SDL_QUIT) {
				not_done = false;
			} else if (Arcollect::gui::window_borders::event(e)) {
				// Propagate event to modals
				auto iter = Arcollect::gui::modal_stack.rbegin();
				while (iter->get().event(e))
					++iter;
			}
		}
		// Check for DB updates
		Arcollect::update_data_version();
		// Update timing informations
		Uint32 new_ticks = SDL_GetTicks();
		time_framedelta = time_now-new_ticks;
		time_now = new_ticks;
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
		renderer->Present();
	}
	// Cleanups
	return 0;
}
