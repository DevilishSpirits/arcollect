#include "sdl2-hpp/SDL.hpp"
#include <arcollect-db-open.hpp>
#include "db/db.hpp"
#include "gui/artwork-collections.hpp"
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
	// Init SDL
	SDL::Hint::SetRenderScaleQuality(SDL::Hint::RENDER_SCALE_QUALITY_BEST);
	if (SDL::Init(SDL::INIT_VIDEO)) {
		std::cerr << "SDL initialization failed: " << SDL::GetError() << std::endl;
		return 1;
	}
	if (SDL::CreateWindowAndRenderer(0,0,SDL_WINDOW_RESIZABLE|SDL_WINDOW_MAXIMIZED,window,renderer)) {
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
	Arcollect::gui::update_background();
	Arcollect::gui::background_slideshow.resize(window_rect);
	Arcollect::gui::modal_stack.push_back(Arcollect::gui::background_slideshow);
	// FIXME Test the grid
	Arcollect::gui::view_vgrid vgrid;
	std::unique_ptr<SQLite3::stmt> grid_stmt;
	Arcollect::database->prepare("SELECT art_artid FROM artworks ORDER BY art_artid;",grid_stmt);
	std::shared_ptr<Arcollect::gui::artwork_collection> simply_all_collection(new Arcollect::gui::artwork_collection_sqlite(grid_stmt));
	vgrid.set_collection(simply_all_collection);
	vgrid.resize(window_rect);
	//Arcollect::gui::modal_stack.push_back(vgrid);
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
