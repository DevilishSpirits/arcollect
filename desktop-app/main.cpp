#include "sdl2-hpp/SDL.hpp"
#include <arcollect-db-open.hpp>
#include "db/db.hpp"
#include "gui/artwork-collections.hpp"
#include "gui/views.hpp"
#include <iostream>
#include <string>


SDL_Window    *window;
extern SDL::Renderer *renderer;
SDL::Renderer *renderer;

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
	// Load the db
	Arcollect::database = Arcollect::db::open();
	// FIXME Try to display a slideshow
	std::shared_ptr<Arcollect::gui::artwork_collection> collection(new Arcollect::gui::artwork_collection_simply_all());
	Arcollect::gui::view_slideshow slideshow;
	slideshow.set_collection(collection);
	slideshow.resize({0,0,100,100});
	// Main-loop
	SDL::Event e;
	bool not_done = true;
	while (not_done) {
		Arcollect::update_data_version();
		if (SDL::WaitEvent(e)) {
			switch (e.type) {
				case SDL_QUIT: {
					not_done = false;
				} break;
				case SDL_WINDOWEVENT: {
					switch (e.window.event) {
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						case SDL_WINDOWEVENT_RESIZED: {
							slideshow.resize({0,0,e.window.data1,e.window.data2});
						} break;
						default: {
							slideshow.event(e);
						} break;
					}
				} break;
				default: {
					slideshow.event(e);
				} break;
			}
		}
		// Render
		renderer->SetDrawColor(0,0,0,0);
		renderer->Clear();
		slideshow.render();
		renderer->Present();
	}
	// Cleanups
	
	return 0;
}
