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
#include <lcms2.h>

extern SDL_Window    *window;
SDL_Window    *window;
extern SDL::Renderer *renderer;
SDL::Renderer *renderer;
extern cmsHPROFILE    cms_screenprofile;
std::vector<std::reference_wrapper<Arcollect::gui::modal>> Arcollect::gui::modal_stack;

bool debug_redraws;
bool debug_icc_profile;
static std::unique_ptr<SQLite3::stmt> preload_artworks_stmt;

bool Arcollect::gui::enabled = false;

// animation.hpp variables
bool   Arcollect::gui::animation_running;
Uint32 Arcollect::gui::time_now;
Uint32 Arcollect::gui::time_framedelta;

int Arcollect::gui::init(void)
{
	// Sample debug flags
	debug_redraws = Arcollect::debug::is_on("redraws");
	debug_icc_profile = Arcollect::debug::is_on("icc-profile");
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
	// Load ICC profile
	size_t icc_profile_size;
	void  *icc_profile_data = SDL_GetWindowICCProfile(window,&icc_profile_size);
	if (icc_profile_data) {
		cms_screenprofile = cmsOpenProfileFromMem(icc_profile_data,icc_profile_size);
		SDL_free(icc_profile_data);
		if (debug_icc_profile) {
			char description[64];
			char manufacturer[64];
			char model[64];
			char copyright[64];
			cmsGetProfileInfoASCII(cms_screenprofile,cmsInfoDescription,cmsNoLanguage,cmsNoCountry,description,sizeof(description));
			cmsGetProfileInfoASCII(cms_screenprofile,cmsInfoManufacturer,cmsNoLanguage,cmsNoCountry,manufacturer,sizeof(manufacturer));
			cmsGetProfileInfoASCII(cms_screenprofile,cmsInfoModel,cmsNoLanguage,cmsNoCountry,model,sizeof(model));
			cmsGetProfileInfoASCII(cms_screenprofile,cmsInfoCopyright,cmsNoLanguage,cmsNoCountry,copyright,sizeof(copyright));
			std::cerr << "Using screen ICC profile for " << manufacturer << " " << model << " (" << copyright << "): " << description << std::endl;
		}
	} else std::cerr << "No screen ICC profile found, color management disabled" << std::endl;
	// Load font
	// TODO Use real font management
	TTF_Init();
	// Set custom borders
	Arcollect::gui::window_borders::init(window);
	// Prepare preload_artworks stmt
	if (Arcollect::database->prepare(Arcollect::db::schema::preload_artworks,preload_artworks_stmt) != SQLITE_OK)
	std::cerr << "Failed to prepare preload_artworks SQL stmt: " << Arcollect::database->errmsg() << std::endl;
	
	// Init background
	Arcollect::gui::update_background(true);
	
	// Start artwork_loader
	Arcollect::db::artwork_loader::start();
	
	return 0;
}
void Arcollect::gui::start(int argc, char** argv)
{
	if (!Arcollect::gui::enabled) {
		// Get window size
		SDL::Rect window_rect{0,0};
		renderer->GetOutputSize(window_rect.w,window_rect.h);
		// Bootstrap the background
		Arcollect::gui::background_slideshow.resize(window_rect);
		Arcollect::gui::modal_stack.push_back(Arcollect::gui::background_slideshow);
		// Show the first run if not done
		if (Arcollect::config::first_run == 0)
			Arcollect::gui::modal_stack.push_back(Arcollect::gui::first_run_modal);
		
		SDL_ShowWindow(window);
	}
	// Handle CLI
	switch (argc) {
		case 3: {
			std::shared_ptr<Arcollect::db::artwork> artwork = Arcollect::db::artwork::query(atoi(argv[2]));
			if (artwork)
				Arcollect::gui::background_slideshow.target_artwork = artwork;
		} //falltrough;
		case 2: {
			Arcollect::gui::update_background(argv[1],true);
		}
	}
	
	Arcollect::gui::time_now = SDL_GetTicks();
	Arcollect::gui::enabled = true;
}
static Uint32 loop_end_ticks;
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
			Uint32  other;
			Uint32  frame;
			decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending;
			
			debug_sample(Uint32 loop_start_ticks, Uint32 event_start_ticks, Uint32 render_start_ticks, Uint32 loader_start_ticks, Uint32 final_ticks, Uint32 loop_end_ticks, decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending) : 
				idle  (event_start_ticks  -   loop_start_ticks),
				event (render_start_ticks -  event_start_ticks),
				render(loader_start_ticks - render_start_ticks),
				loader(final_ticks        - loader_start_ticks),
				other (loop_start_ticks   -     loop_end_ticks),
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
				result.other  = std::max(other ,right.other );
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
				draw_time_bar(time_bar,other ,0  ,0  ,255);
			}
			std::string print(void) {
				return "	Idle  : " + std::to_string(idle  ) + "ms\n"
				+ "	Event : " + std::to_string(event ) + "ms\n"
				+ "	Render: " + std::to_string(render) + "ms\n"
				+ "	Loader: " + std::to_string(loader) + "ms " + std::to_string(load_pending) + " artworks pending\n"
				+ "	Other : " + std::to_string(other ) + "ms"
					#ifdef WITH_XDG
					+ " (D-Bus)\n"
					#else
					+ "\n"
					#endif
				+ "	Total = " + std::to_string(frame ) + "ms/"+std::to_string(1000.f/frame)+"FPS\n"
				;
			}
		};
		debug_sample frame_sample(loop_start_ticks,event_start_ticks,render_start_ticks,loader_start_ticks,final_ticks,loop_end_ticks,load_pending_count);
		
		static Uint32 last_second_tick = 0;
		static debug_sample last_second_samples[30];
		const auto last_second_samples_n = sizeof(last_second_samples)/sizeof(last_second_samples[0]);
		static int last_second_sample_i = 0;
		if (last_second_tick != final_ticks/100) {
			last_second_tick    = final_ticks/100;
			last_second_sample_i++;
			last_second_sample_i %= last_second_samples_n;
			last_second_samples[last_second_sample_i] = debug_sample(0,0,0,0,0,0,0);
		}
		last_second_samples[last_second_sample_i] = last_second_samples[last_second_sample_i].max(frame_sample);
		debug_sample maximums = debug_sample(0,0,0,0,0,0,0);
		for (const debug_sample &sample: last_second_samples)
			maximums = maximums.max(sample);
		
		std::string stats = "Tick: " + std::to_string(final_ticks) + "\n"
			+ "Frame stats:\n"+ frame_sample.print()
			+ "Maximums (last 3 seconds):\n"+ maximums.print()
			+ "\n"
			//+ "animation_running:" + (saved_animation_running ? "y" : "n")
			+ "Image memory usage: " + std::to_string(Arcollect::db::artwork_loader::image_memory_usage >> 20) +" MiB"
		;
		std::cerr << stats << std::endl;
		// Render debug window
		Arcollect::gui::font::Renderable stats_text(stats.c_str(),14,800);
		static constexpr const SDL::Point stats_text_tl{5,20};
		SDL::Rect debug_box_dstrect{0,0,stats_text.size().x+5+stats_text_tl.x,stats_text.size().y+5+stats_text_tl.y};
		renderer->SetDrawColor(0,0,0,224);
		renderer->FillRect(debug_box_dstrect);
		stats_text.render_tl(stats_text_tl);
		
		// Render time bar
		frame_sample.draw_time_bar({0,0,0,4}); // Draw frame time
		maximums.draw_time_bar({0,11,0,1});    // Draw last 3 seconds max
		maximums = debug_sample(0,0,0,0,0,0,0);
		for (int i = 0; i < 1; i++)
			maximums = maximums.max(last_second_samples[(i+last_second_sample_i)%last_second_samples_n]);
		maximums.draw_time_bar({0,5,0,1});    // Draw last 0.1 second max
		for (int i = 1; i < 10; i++)
			maximums = maximums.max(last_second_samples[(i+last_second_sample_i)%last_second_samples_n]);
		maximums.draw_time_bar({0,7,0,1});    // Draw last second max
		for (int i = 10; i < 20; i++)
			maximums = maximums.max(last_second_samples[(i+last_second_sample_i)%last_second_samples_n]);
		maximums.draw_time_bar({0,9,0,1});    // Draw last 2 seconds max
		// Draw 60FPS mark
		renderer->SetDrawColor(0,255,0,192);
		renderer->DrawLine(1000.f/60,0,1000.f/60,7);
		// Draw 30FPS mark
		renderer->SetDrawColor(255,255,0,192);
		renderer->DrawLine(1000.f/30,0,1000.f/30,7);
	}
	loop_end_ticks = SDL_GetTicks();
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

void Arcollect::gui::wakeup_main(void)
{
	SDL_Event e;
	e.type = SDL_USEREVENT;
	SDL_PushEvent(&e);
}
