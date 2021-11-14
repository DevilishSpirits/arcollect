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
#include "../i18n.hpp"
#include "../art-reader/image.hpp"
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
#include <arcollect-debug.hpp>
#include <arcollect-sqls.hpp>
#include <iostream>
#if WITH_XDG
#include <stdlib.h> // For setenv()
#endif

extern SDL_Window    *window;
SDL_Window    *window;
extern SDL::Renderer *renderer;
SDL::Renderer *renderer;

#include "sqlite-busy-handler.cpp"

static std::unique_ptr<SQLite3::stmt> preload_artworks_stmt;
static int window_screen_index;

bool Arcollect::gui::enabled = false;

// animation.hpp variables
bool   Arcollect::gui::animation_running;
Uint32 Arcollect::gui::time_now;
Uint32 Arcollect::gui::time_framedelta;

int Arcollect::gui::init(void)
{
	// Init SDL
	SDL::Hint::SetRenderScaleQuality(SDL::Hint::RENDER_SCALE_QUALITY_BEST);
	#if WITH_XDG
	setenv("SDL_VIDEO_X11_WMCLASS",ARCOLLECT_X11_WM_CLASS_STR,false);
	#endif
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
	Arcollect::art_reader::set_screen_icc_profile(window);
	// Set custom borders
	Arcollect::gui::window_borders::init(window);
	// Prepare preload_artworks stmt
	if (Arcollect::database->prepare(Arcollect::db::sql::preload_artworks,preload_artworks_stmt) != SQLITE_OK)
	std::cerr << "Failed to prepare preload_artworks SQL stmt: " << Arcollect::database->errmsg() << std::endl;
	
	// Init background
	Arcollect::gui::update_background(true);
	
	// Setup database
	sqlite3_busy_handler((sqlite3*)Arcollect::database.get(),Arcollect::sqlite_busy::handler,NULL);
	
	// Load system language
	Arcollect::i18n_strings::set_locale_system();
	
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
	window_screen_index =  SDL_GetWindowDisplayIndex(window);
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
	// Get window size
	SDL::Rect window_rect{0,0};
	renderer->GetOutputSize(window_rect.w,window_rect.h);
	// Handle event
	Uint32 event_start_ticks = SDL_GetTicks();
	while (has_event) {
		if (Arcollect::gui::window_borders::event(e)) {
			// Propagate event to modals
			auto iter = Arcollect::gui::modal_stack.rbegin();
			while (Arcollect::gui::modal_get(iter).event(e,window_rect))
				++iter;
			// Pop marked modals
			for (auto iter = Arcollect::gui::modal_stack.begin(); iter != Arcollect::gui::modal_stack.end(); ++iter) {
				auto &modal = Arcollect::gui::modal_get(iter);
				if (modal.to_pop) {
					modal.to_pop = false;
					auto iter_copy = iter;
					++iter;
					Arcollect::gui::modal_stack.erase(iter_copy);
				}
			}
		}
		switch (e.type) {
			case SDL_LOCALECHANGED: {
				Arcollect::i18n_strings::set_locale_system();
			} break;
			case SDL_QUIT: {
			} return false;
		}
		// Process other events
		has_event = SDL::PollEvent(e);
	}
	// Check for DB updates
	sqlite_int64 data_version_snapshot = Arcollect::data_version;
	if (data_version_snapshot != Arcollect::update_data_version()) {
		// Query artworks to preload
		if (preload_artworks_stmt) {
			preload_artworks_stmt->reset();
			while (preload_artworks_stmt->step() == SQLITE_ROW)
				Arcollect::db::artwork::query(preload_artworks_stmt->column_int64(0))->queue_for_load();
		}
	}
	// Check for screen change
	int current_window_screen_index = SDL_GetWindowDisplayIndex(window);
	if (window_screen_index != current_window_screen_index) {
		Arcollect::art_reader::set_screen_icc_profile(window);
		window_screen_index = current_window_screen_index;
	}
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
		Arcollect::gui::modal_get(iter).render(window_rect);
	}
	Arcollect::gui::window_borders::render();
	Uint32 loader_start_ticks = SDL_GetTicks();
	decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending_count;
	static decltype(Arcollect::db::artwork_loader::done) main_done;
	// Try to load requested artworks into VRAM
	for (auto &artwork: Arcollect::db::artwork_loader::pending_main) {
		auto iter = main_done.find(artwork);
		if (iter != main_done.end()) {
			std::unique_ptr<SDL::Texture> text(SDL::Texture::CreateFromSurface(renderer,iter->second.get()));
			artwork->texture_loaded(text);
			main_done.erase(iter);
			if (SDL_GetTicks()-render_start_ticks > 40)
				break;
		}
	}
	// Append pending_main to pending_thread and steal list of loaded artworks
	{
		using namespace Arcollect::db; // To shorten lines
		std::lock_guard<std::mutex> lock_guard(Arcollect::db::artwork_loader::lock);
		// Steal Arcollect::db::artwork_loader::done
		main_done.merge(artwork_loader::done);
		// Queue pending artworks in pending_thread_second
		for (auto &artwork: artwork_loader::pending_main)
			if (artwork->load_state == artwork->LOAD_SCHEDULED) {
				artwork_loader::pending_thread_second.emplace_back(artwork);
				artwork->load_state = artwork->LOAD_PENDING;
			}
		// Move pending_main in pending_thread_first
		artwork_loader::pending_thread_first = std::move(artwork_loader::pending_main);
		// Snapshot load_pending_count
		load_pending_count = artwork_loader::pending_thread_second.size();
	}
	Arcollect::db::artwork_loader::condition_variable.notify_all();
	// Load artworks
	if (main_done.size()) {
		// Generate a redraw
		Arcollect::gui::animation_running = true;
		// Load arts
		do {
			decltype(main_done)::node_type art = main_done.extract(main_done.begin());
			std::unique_ptr<SDL::Texture> text(SDL::Texture::CreateFromSurface(renderer,art.mapped().get()));
			art.key()->texture_loaded(text);
			// Ensure nice framerate 
		} while ((SDL_GetTicks()-render_start_ticks < 50) && main_done.size());
	}
	// Unload artworks if exceeding image_memory_limit
	while (Arcollect::db::artwork_loader::image_memory_usage>>20 > static_cast<std::size_t>(Arcollect::config::image_memory_limit)) {
		Arcollect::db::artwork& artwork = *--Arcollect::db::artwork::last_rendered.end();
		artwork.texture_unload();
	}
	// Redraws debugging
	if (Arcollect::debug.redraws) {
		Uint32 final_ticks = SDL_GetTicks();
		static constexpr SDL::Color   idle_color = 0xFFFFFFff; // White
		static constexpr SDL::Color  event_color = 0xFFFF00ff; // Yellow
		static constexpr SDL::Color render_color = 0x00FFFFff; // Cyan
		static constexpr SDL::Color loader_color = 0xFF0000ff; // Red
		static constexpr SDL::Color  other_color = 0x0000FFff; // Blue
		static constexpr SDL::Color  total_color = 0xFFFFFFff; // White
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
			static inline void draw_time_bar(SDL::Rect &time_bar, Uint32 var, const SDL::Color &color) {
				time_bar.w  = var;
				renderer->SetDrawColor(color.r,color.g,color.b,color.a);
				renderer->FillRect(time_bar);
				time_bar.x += var;
			}
			void draw_time_bar(SDL::Rect time_bar) {
				draw_time_bar(time_bar,event ,event_color);
				draw_time_bar(time_bar,render,render_color);
				draw_time_bar(time_bar,loader,loader_color);
				draw_time_bar(time_bar,other ,other_color);
			}
			std::string print(void) {
				return "	Idle  : " + std::to_string(idle  ) + "ms\n"
				+      "	Event : " + std::to_string(event ) + "ms\n"
				+      "	Render: " + std::to_string(render) + "ms\n"
				+      "	Loader: " + std::to_string(loader) + "ms " + std::to_string(load_pending) + " artworks pending\n"
				+      "	Other : " + std::to_string(other ) + "ms"
					#ifdef WITH_XDG
					+ " (D-Bus)\n"
					#else
					+ "\n"
					#endif
				+ "	Total = " + std::to_string(frame ) + "ms/"+std::to_string(1000.f/frame)+"FPS\n"
				;
			}
			void print(Arcollect::gui::font::Elements &elements) {
				elements <<   idle_color << U"	Idle  : "sv << std::to_string(idle  ) << U"ms\n"sv
				         <<  event_color << U"	Event : "sv << std::to_string(event ) << U"ms\n"sv
				         << render_color << U"	Render: "sv << std::to_string(render) << U"ms\n"sv
				         << loader_color << U"	Loader: "sv << std::to_string(loader) << U"ms "sv << std::to_string(load_pending) << U" artworks pending\n"sv
				         <<  other_color << U"	Other : "sv << std::to_string(other ) << U"ms "sv
				         #ifdef WITH_XDG
				         	" (D-Bus)\n"
				         #else
				         	"\n"
				         	#endif
				         <<  total_color << U"	Total = "sv << std::to_string(frame ) << U"ms/"sv << std::to_string(1000.f/frame) << U"FPS\n"sv
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
		
		std::cerr << "Tick: " << final_ticks << "\nFrame stats:\n" << frame_sample.print()
			<< "Maximums (last 3 seconds):\n" << maximums.print()
			<< "\n"
			//<< "animation_running:" << (saved_animation_running ? "y" : "n") << "\n"
			<< "Image memory usage: " << std::to_string(Arcollect::db::artwork_loader::image_memory_usage >> 20) <<" MiB"
			<< std::endl;
		// Generate debug window text
		Arcollect::gui::font::Elements stats_elements;
		//stats_elements.initial_height() = 14;
		stats_elements << U"Tick: "sv << std::to_string(final_ticks) << U"\nFrame stats:\n"sv;
		frame_sample.print(stats_elements);
		stats_elements << U"Maximums (last 3 seconds):\n"sv;
		maximums.print(stats_elements);
		stats_elements << U"Image memory usage: "sv << std::to_string(Arcollect::db::artwork_loader::image_memory_usage >> 20) << U" MiB"sv;
		
		// Render debug window text
		Arcollect::gui::font::Renderable stats_text(stats_elements,800);
		static constexpr SDL::Point stats_text_tl{5,20};
		SDL::Rect debug_box_dstrect{0,0,stats_text.size().x+10+stats_text_tl.x,stats_text.size().y+5+stats_text_tl.y};
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
		Arcollect::db::artwork_loader::pending_thread_first.clear();
		Arcollect::db::artwork_loader::pending_thread_second.clear();
	}
	Arcollect::gui::enabled = false;
}

void Arcollect::gui::wakeup_main(void)
{
	SDL_Event e;
	e.type = SDL_USEREVENT;
	SDL_PushEvent(&e);
}
