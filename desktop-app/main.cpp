#include "sdl2-hpp/SDL.hpp"
#include <arcollect-db-open.hpp>
#include "db/db.hpp"
#include "gui/artwork-collections.hpp"
#include "gui/slideshow.hpp"
#include "gui/views.hpp"
#include "gui/font.hpp"
#include <iostream>
#include <string>


SDL_Window    *window;
extern SDL::Renderer *renderer;
SDL::Renderer *renderer;

Uint32 time_now;
Uint32 time_framedelta;

int main(void)
{
	// Init SDL
	SDL::Hint::SetRenderScaleQuality(SDL::Hint::RENDER_SCALE_QUALITY_BEST);
	if (SDL::Init(SDL::INIT_VIDEO)) {
		std::cerr << "SDL initialization failed: " << SDL::GetError() << std::endl;
		return 1;
	}
	if (SDL::CreateWindowAndRenderer(100,100,SDL_WINDOW_RESIZABLE,window,renderer)) {
		std::cerr << "Failed to create window: " << SDL::GetError() << std::endl;
		return 1;
	}
	// Get window size
	SDL::Rect window_rect{0,0};
	renderer->GetOutputSize(window_rect.w,window_rect.h);
	// Load font
	// TODO Use real font management
	TTF_Init();
	// Load the db
	Arcollect::database = Arcollect::db::open();
	// Bootstrap the background
	Arcollect::gui::update_background();
	Arcollect::gui::background_slideshow.resize(window_rect);
	// Main-loop
	SDL::Event e;
	bool not_done = true;
	time_now = SDL_GetTicks();
	while (not_done) {
		if (SDL::WaitEvent(e)) {
			switch (e.type) {
				case SDL_QUIT: {
					not_done = false;
				} break;
				case SDL_WINDOWEVENT: {
					switch (e.window.event) {
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							Arcollect::gui::background_slideshow.resize({0,0,e.window.data1,e.window.data2});
						} break;
						default: {
						} break;
					}
				} break;
				default: {
				} break;
			}
		}
		// Check for DB updates
		Arcollect::update_data_version();
		// Update timing informations
		Uint32 new_ticks = SDL_GetTicks();
		time_framedelta = time_now-new_ticks;
		time_now = new_ticks;
		// Render the background slideshow
		renderer->SetDrawColor(0,0,0,0);
		renderer->Clear();
		Arcollect::gui::background_slideshow.render();
		renderer->Present();
	}
	// Cleanups
	return 0;
}
