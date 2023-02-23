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
#include "../sdl2-hpp/SDL.hpp"
#include <OpenImageIO/imageio.h> // Enable some stuff
#undef main // This cause name clash
#include "animation.hpp"
#include "font.hpp"
#include "modal.hpp"
#include "slideshow.hpp"
#include "time.hpp"
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
Arcollect::time_point Arcollect::frame_time;
unsigned int Arcollect::frame_number = 0;

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
	// Configure OpenImageIO
	OIIO::attribute("threads",1); // Disable OIIO threading system
	// Init font system
	Arcollect::gui::font::init();
	// Load system language
	Arcollect::set_locale_system();
	// Load ICC profile
	Arcollect::art_reader::set_screen_icc_profile(window);
	// Set custom borders
	Arcollect::gui::window_borders::init(window);
	// Prepare preload_artworks stmt
	if (Arcollect::database->prepare(Arcollect::db::sql::preload_artworks,preload_artworks_stmt) != SQLITE_OK)
	std::cerr << "Failed to prepare preload_artworks SQL stmt: " << Arcollect::database->errmsg() << std::endl;
	
	// Setup database
	sqlite3_busy_handler((sqlite3*)Arcollect::database.get(),Arcollect::sqlite_busy::handler,NULL);
	
	return 0;
}
void Arcollect::gui::start(int argc, char** argv)
{
	if (!Arcollect::gui::enabled) {
		// Get window size
		SDL::Rect window_rect{0,0};
		renderer->GetOutputSize(window_rect.w,window_rect.h);
		// Bootstrap the background
		Arcollect::gui::modal_stack.push_back(Arcollect::gui::background_slideshow);
		
		SDL_ShowWindow(window);
	}
	Arcollect::gui::update_background("");
	// Handle CLI
	switch (argc) {
		case 3: {
			std::shared_ptr<Arcollect::db::artwork> artwork = Arcollect::db::artwork::query(atoi(argv[2]));
			if (artwork)
				Arcollect::gui::background_slideshow.target_artwork = artwork;
		} //falltrough;
		case 2: {
			Arcollect::gui::update_background(argv[1]);
		}
	}
	
	Arcollect::frame_time = Arcollect::frame_clock::now();
	Arcollect::gui::enabled = true;
	window_screen_index =  SDL_GetWindowDisplayIndex(window);
}
static Arcollect::time_point loop_end_ticks;
bool Arcollect::gui::main(void)
{
	SDL::Event e;
	const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
	Arcollect::time_point loop_start_ticks = Arcollect::frame_clock::now();
	// Wait for event
	bool saved_animation_running = Arcollect::gui::animation_running;
	Arcollect::gui::animation_running = false;
	bool has_event = saved_animation_running ? SDL::PollEvent(e) : SDL::WaitEvent(e);
	// Check for emergency SFW shortcut (Ctrl+Maj+X)
	if ((SDL_GetModState()&(KMOD_CTRL|KMOD_SHIFT)) && keyboard_state[SDL_GetScancodeFromKey(SDLK_x)])
		Arcollect::set_filter_rating(Arcollect::config::RATING_NONE);
	// Update timing informations
	Arcollect::time_point new_ticks = Arcollect::frame_clock::now();
	Arcollect::frame_time = new_ticks;
	Arcollect::frame_number++;
	// Compute render context
	Arcollect::gui::modal::render_context render_ctx{*renderer};
	renderer->GetOutputSize(render_ctx.window_size);
	render_ctx.target.x = render_ctx.target.y = render_ctx.titlebar_target.x = render_ctx.titlebar_target.y = 0;
	render_ctx.target.w = render_ctx.titlebar_target.w = render_ctx.window_size.x;
	render_ctx.target.h = render_ctx.window_size.y;
	render_ctx.titlebar_target.h = Arcollect::gui::window_borders::title_height;
	// Handle event
	Arcollect::time_point event_start_ticks = Arcollect::frame_clock::now();
	while (has_event) {
		if (Arcollect::gui::window_borders::event(e)) {
			// Propagate event to modals
			auto iter = Arcollect::gui::modal_stack.rbegin();
			while (Arcollect::gui::modal_get(iter).event(e,render_ctx))
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
				Arcollect::set_locale_system();
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
				Arcollect::db::artwork::query(preload_artworks_stmt->column_int64(0))->get_thumbnail()->queue_for_load();
		}
	}
	// Check for screen change
	int current_window_screen_index = SDL_GetWindowDisplayIndex(window);
	if (window_screen_index != current_window_screen_index) {
		Arcollect::art_reader::set_screen_icc_profile(window);
		window_screen_index = current_window_screen_index;
	}
	Arcollect::time_point render_start_ticks = Arcollect::frame_clock::now();
	// Render frame
	renderer->SetDrawColor(0,0,0,0);
	renderer->Clear();
	for (auto& iter: Arcollect::gui::modal_stack) {
		// Draw a backdrop
		renderer->SetDrawBlendMode(SDL::BLENDMODE_BLEND);
		renderer->SetDrawColor(0,0,0,128);
		renderer->FillRect();
		// Render
		Arcollect::gui::modal_get(iter).render(render_ctx);
	}
	Arcollect::gui::window_borders::render(render_ctx);
	Arcollect::time_point loader_start_ticks = Arcollect::frame_clock::now();
	decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending_count;
	static decltype(Arcollect::db::artwork_loader::done) main_done;
	// Try to load requested artworks into VRAM
	for (auto &artwork: Arcollect::db::artwork_loader::pending_main) {
		auto node = main_done.extract(artwork);
		if (node) {
			node.value()->load_stage_two(*renderer);
			if (Arcollect::frame_clock::now()-render_start_ticks > std::chrono::milliseconds(40))
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
				artwork->load_state = artwork->LOAD_PENDING_STAGE1;
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
			art.value()->load_stage_two(*renderer);
			// Ensure nice framerate 
		} while ((Arcollect::frame_clock::now()-render_start_ticks < std::chrono::milliseconds(50)) && main_done.size());
	}
	// Unload artworks not seen recently
	while (!Arcollect::db::download::last_rendered.empty()) {
		Arcollect::db::download& artwork = *--Arcollect::db::download::last_rendered.end();
		if (!artwork.keep_loaded())
			artwork.unload();
		else break;
	}
	// Redraws debugging
	if (Arcollect::debug.redraws) {
		Arcollect::time_point final_ticks = Arcollect::frame_clock::now();
		static constexpr SDL::Color   idle_color = 0xFFFFFFff; // White
		static constexpr SDL::Color  event_color = 0xFFFF00ff; // Yellow
		static constexpr SDL::Color render_color = 0x00FFFFff; // Cyan
		static constexpr SDL::Color loader_color = 0xFF0000ff; // Red
		static constexpr SDL::Color  other_color = 0x0000FFff; // Blue
		static constexpr SDL::Color  total_color = 0xFFFFFFff; // White
		struct debug_sample {
			using duration = Arcollect::frame_clock::duration;
			duration   idle;
			duration  event;
			duration render;
			duration loader;
			duration  other;
			duration  frame;
			decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending;
			
			constexpr debug_sample(Arcollect::time_point loop_start_ticks, Arcollect::time_point event_start_ticks, Arcollect::time_point render_start_ticks, Arcollect::time_point loader_start_ticks, Arcollect::time_point final_ticks, Arcollect::time_point loop_end_ticks, decltype(Arcollect::db::artwork_loader::pending_main)::size_type load_pending) : 
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
			constexpr static debug_sample zero(void) {
				return debug_sample(Arcollect::time_point::min(),Arcollect::time_point::min(),Arcollect::time_point::min(),Arcollect::time_point::min(),Arcollect::time_point::min(),Arcollect::time_point::min(),0);
			}
			
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
			void draw_histo_section(SDL::Point &origin, duration time, const SDL::Color &color) const {
				int var = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
				renderer->SetDrawColor(color);
				renderer->DrawLine(origin,{origin.x,origin.y-var});
				origin.y -= var;
			}
			void draw_histo_bar(SDL::Point origin) {
				draw_histo_section(origin,event ,event_color);
				draw_histo_section(origin,render,render_color);
				draw_histo_section(origin,loader,loader_color);
				draw_histo_section(origin,other ,other_color);
			}
			
			static inline void draw_time_bar(SDL::Rect &time_bar, duration time, const SDL::Color &color) {
				int var = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
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
			static std::string duration_to_string(duration time) {
				return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(time).count()).append("ms");
			}
			void print(Arcollect::gui::font::Elements &elements) {
				constexpr float fps_numerator = static_cast<float>(decltype(frame)::period::den)/static_cast<float>(decltype(frame)::period::num);
				elements <<   idle_color << U"	Idle  : "sv << duration_to_string(idle  ) << U"\n"sv
				         <<  event_color << U"	Event : "sv << duration_to_string(event ) << U"\n"sv
				         << render_color << U"	Render: "sv << duration_to_string(render) << U"\n"sv
				         << loader_color << U"	Loader: "sv << duration_to_string(loader) << U" "sv << std::to_string(load_pending) << U" artworks pending\n"sv
				         <<  other_color << U"	Other : "sv << duration_to_string(other ) << U" "sv
				         #ifdef WITH_XDG
				         	" (D-Bus)\n"
				         #else
				         	"\n"
				         	#endif
				         <<  total_color << U"	Total = "sv << duration_to_string(frame ) << U"/"sv << std::to_string(fps_numerator/frame.count()) << U"FPS\n"sv
				;
			}
		};
		debug_sample frame_sample(loop_start_ticks,event_start_ticks,render_start_ticks,loader_start_ticks,final_ticks,loop_end_ticks,load_pending_count);
		
		static unsigned int last_second_tick = 0;
		static debug_sample last_second_samples[600];
		const auto last_second_samples_n = sizeof(last_second_samples)/sizeof(last_second_samples[0]);
		static int last_second_sample_i = 0;
		unsigned int final_ticks_int = static_cast<unsigned int>(std::chrono::time_point_cast<std::chrono::milliseconds>(final_ticks).time_since_epoch().count()/10);
		if (last_second_tick != final_ticks_int) {
			last_second_tick    = final_ticks_int;
			last_second_sample_i++;
			last_second_sample_i %= last_second_samples_n;
			last_second_samples[last_second_sample_i] = debug_sample::zero();
		}
		last_second_samples[last_second_sample_i] = last_second_samples[last_second_sample_i].max(frame_sample);
		debug_sample maximums = debug_sample::zero();
		for (const debug_sample &sample: last_second_samples)
			maximums = maximums.max(sample);
		
		// Print histogram
		for (std::decay<decltype(last_second_samples_n)>::type i = 0; i < last_second_samples_n; ++i) {
			SDL::Point origin{static_cast<int>((i-last_second_sample_i+last_second_samples_n)%last_second_samples_n),render_ctx.target.h};
			if (!(i % (last_second_samples_n/10))) {
				renderer->SetDrawColor(0xFFFFFFFF);
				renderer->DrawLine(origin,{origin.x,origin.y-100});
			}
		}
		// Generate debug window text
		Arcollect::gui::font::Elements stats_elements;
		//stats_elements.initial_height() = 14;
		stats_elements << U"Frame number: "sv << std::to_string(Arcollect::frame_number) << U"\nTick: "sv << std::to_string(final_ticks.time_since_epoch().count()) << U"\nFrame stats:\n"sv;
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
		maximums = debug_sample::zero();
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
	loop_end_ticks = Arcollect::frame_clock::now();
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
