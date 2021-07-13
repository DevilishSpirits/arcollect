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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "main.hpp"
#include "../config.hpp"
#include "../db/artwork-loader.hpp"
#include "../db/db.hpp"
#include "../db/filter.hpp"
#include "../sdl2-hpp/SDL.hpp"
#undef main // This cause name clash
#include "animation.hpp"
#include "first-run.hpp"
#include "font.hpp"
#include "modal.hpp"
#include "slideshow.hpp"
#include "window-borders.hpp"
#include <arcollect-db-schema.hpp>
#include <arcollect-debug.hpp>
#include <iostream>
#include <vector>

extern SDL_Window    *window;
SDL_Window    *window;
extern SDL::Renderer *renderer;
SDL::Renderer *renderer;
std::vector<std::reference_wrapper<Arcollect::gui::modal>> Arcollect::gui::modal_stack;

bool debug_redraws;
static std::unique_ptr<SQLite3::stmt> preload_artworks_stmt;

bool Arcollect::gui::enabled = false;

// animation.hpp variables
bool   Arcollect::gui::animation_running;
Uint32 Arcollect::gui::time_now;
Uint32 Arcollect::gui::time_framedelta;

int Arcollect::gui::init(void)
{
	// Sample "redraws" debug flag
	debug_redraws = Arcollect::debug::is_on("redraws");
	// Init SDL
	SDL::Hint::SetRenderScaleQuality(SDL::Hint::RENDER_SCALE_QUALITY_BEST);
	if (SDL::Init(SDL::INIT_VIDEO)) {
		std::cerr << "SDL initialization failed: " << SDL::GetError() << std::endl;
		return 1;
	}
	// Create the SDL window
	int window_create_flags = SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIDDEN;
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
	// Load font
	// TODO Use real font management
	TTF_Init();
	// Set custom borders
	Arcollect::gui::window_borders::init(window);
	// Prepare preload_artworks stmt
	if (Arcollect::database->prepare(Arcollect::db::schema::preload_artworks,preload_artworks_stmt) != SQLITE_OK)
	std::cerr << "Failed to prepare preload_artworks SQL stmt: " << Arcollect::database->errmsg() << std::endl;
	
	
	// Start artwork_loader
	Arcollect::db::artwork_loader::start();
	
	return 0;
}
void Arcollect::gui::start(int argc, char** argv)
{
	// Get window size
	SDL::Rect window_rect{0,0};
	renderer->GetOutputSize(window_rect.w,window_rect.h);
	// Bootstrap the background
	Arcollect::gui::update_background(true);
	Arcollect::gui::background_slideshow.resize(window_rect);
	Arcollect::gui::modal_stack.push_back(Arcollect::gui::background_slideshow);
	// Show the first run if not done
	if (Arcollect::config::first_run == 0)
		Arcollect::gui::modal_stack.push_back(Arcollect::gui::first_run_modal);
	
	SDL_ShowWindow(window);
	
	Arcollect::gui::time_now = SDL_GetTicks();
	Arcollect::gui::enabled = true;
}
bool Arcollect::gui::main(void)
{
	SDL::Event e;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	Uint32 loop_start_ticks = SDL_GetTicks();
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
	Uint32 event_start_ticks = SDL_GetTicks();
	while (has_event) {
		if (Arcollect::gui::window_borders::event(e)) {
			// Propagate event to modals
			auto iter = Arcollect::gui::modal_stack.rbegin();
			while (iter->get().event(e))
				++iter;
		}
		if (e.type == SDL_QUIT)
			return false;
		// Process other events
		has_event = SDL::PollEvent(e);
	}
	// Check for DB updates
	Arcollect::update_data_version();
	Uint32 render_start_ticks = SDL_GetTicks();
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
	Uint32 loader_start_ticks = SDL_GetTicks();
	auto load_pending_count = Arcollect::db::artwork_loader::pending_main.size();
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
				Arcollect::db::artwork_loader::image_memory_usage += art.first->image_memory();
			}
			Arcollect::db::artwork_loader::done.clear();
		}
		// Update pending list
		if (Arcollect::db::artwork_loader::pending_main.size())
			Arcollect::db::artwork_loader::pending_thread = std::move(Arcollect::db::artwork_loader::pending_main);
	}
	// Unload artworks if exceeding image_memory_limit
	while (Arcollect::db::artwork_loader::image_memory_usage>>20 > Arcollect::config::image_memory_limit) {
		Arcollect::db::artwork& artwork = *--Arcollect::db::artwork::last_rendered.end();
		Arcollect::db::artwork_loader::image_memory_usage -= artwork.image_memory();
		artwork.texture_unload();
	}
	// Query artworks to preload
	if (preload_artworks_stmt) {
		preload_artworks_stmt->reset();
		while (preload_artworks_stmt->step() == SQLITE_ROW)
			Arcollect::db::artwork::query(preload_artworks_stmt->column_int64(0))->queue_for_load();
	}
	// Redraws debugging
	if (debug_redraws) {
		Uint32 final_ticks = SDL_GetTicks();
		struct debug_sample {
			Uint32   idle;
			Uint32  event;
			Uint32 render;
			Uint32 loader;
			Uint32  frame;
			decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending;
			
			debug_sample(Uint32 loop_start_ticks, Uint32 event_start_ticks, Uint32 render_start_ticks, Uint32 loader_start_ticks, Uint32 final_ticks, decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending) : 
				idle  (event_start_ticks  -   loop_start_ticks),
				event (render_start_ticks -  event_start_ticks),
				render(loader_start_ticks - render_start_ticks),
				loader(final_ticks        - loader_start_ticks),
				frame (final_ticks        -  event_start_ticks),
				load_pending(load_pending)
			{
			}
			debug_sample(void) = default;
			
			debug_sample max(const debug_sample &right) {
				debug_sample result;
				result.idle   = std::max(idle  ,right.idle  );
				result.event  = std::max(event ,right.event );
				result.render = std::max(render,right.render);
				result.loader = std::max(loader,right.loader);
				result.frame  = std::max(frame ,right.frame );
				result.load_pending = std::max(load_pending,right.load_pending);
				return result;
			}
			static inline void draw_time_bar(SDL::Rect &time_bar, Uint32 var, Uint8 r, Uint8 g, Uint8 b) {
				time_bar.w  = var;
				renderer->SetDrawColor(r,g,b,255);
				renderer->FillRect(time_bar);
				time_bar.x += var;
			}
			void draw_time_bar(SDL::Rect time_bar) {
				draw_time_bar(time_bar,event ,255,255,0  );
				draw_time_bar(time_bar,render,0  ,255,255);
				draw_time_bar(time_bar,loader,255,0  ,0  );
			}
			std::string print(void) {
				return "	Idle  : " + std::to_string(idle  ) + "ms\n"
				+ "	Event : " + std::to_string(event ) + "ms\n"
				+ "	Render: " + std::to_string(render) + "ms\n"
				+ "	Loader: " + std::to_string(loader) + "ms " + std::to_string(load_pending) + " artworks pending\n"
				+ "	Total = " + std::to_string(frame ) + "ms/"+std::to_string(1000.f/frame)+"FPS\n"
				;
			}
		};
		debug_sample frame_sample(loop_start_ticks,event_start_ticks,render_start_ticks,loader_start_ticks,final_ticks,load_pending_count);
		
		static Uint32 last_second_tick = 0;
		static debug_sample last_second_sample;
		constexpr const auto last_second_reset_interval = 3000;
		if (last_second_tick != final_ticks/last_second_reset_interval) {
			last_second_tick    = final_ticks/last_second_reset_interval;
			last_second_sample  = debug_sample(0,0,0,0,0,0);
		}
		last_second_sample = last_second_sample.max(frame_sample);
		
		std::string stats = "Tick: " + std::to_string(final_ticks) + "\n"
			+ "Frame stats:\n"+ frame_sample.print()
			+ "Maximums (reset in " + std::to_string((last_second_reset_interval-final_ticks)%last_second_reset_interval) + "ms):\n"+ last_second_sample.print()
			+ "\n"
			//+ "animation_running:" + (saved_animation_running ? "y" : "n")
			+ "Image memory usage: " + std::to_string(Arcollect::db::artwork_loader::image_memory_usage >> 20) +" MiB"
		;
		std::cerr << stats << std::endl;
		// Render debug window
		Arcollect::gui::Font font;
		Arcollect::gui::TextPar debug_par(font,stats,14);
		SDL::Texture* debug_par_text(debug_par.render(800));
		SDL::Point debug_par_size;
		debug_par_text->QuerySize(debug_par_size);
		SDL::Rect debug_par_text_dstrect{5,10,debug_par_size.x,debug_par_size.y};
		SDL::Rect debug_box_dstrect{0,0,debug_par_text_dstrect.w+5+debug_par_text_dstrect.x,debug_par_text_dstrect.h+5+debug_par_text_dstrect.y};
		renderer->SetDrawBlendMode(SDL::BLENDMODE_BLEND);
		renderer->SetDrawColor(0,0,0,224);
		renderer->FillRect(debug_box_dstrect);
		renderer->Copy(debug_par_text,NULL,&debug_par_text_dstrect);
		
		// Render time bar
		frame_sample.draw_time_bar({0,0,0,4});
		last_second_sample.draw_time_bar({0,4,0,1});
		// Draw 60FPS mark
		renderer->SetDrawColor(0,255,0,192);
		renderer->DrawLine(1000.f/60,0,1000.f/60,7);
		// Draw 30FPS mark
		renderer->SetDrawColor(255,255,0,192);
		renderer->DrawLine(1000.f/30,0,1000.f/30,7);
	}
	renderer->Present();
	return true;
}
void Arcollect::gui::stop(void)
{
	Arcollect::gui::modal_stack.clear();
	SDL_HideWindow(window);
	// Erase artwork_loader pending list
	{
		std::lock_guard<std::mutex> lock_guard(Arcollect::db::artwork_loader::lock);
		Arcollect::db::artwork_loader::pending_thread.clear();
	}
	Arcollect::gui::enabled = false;
}
