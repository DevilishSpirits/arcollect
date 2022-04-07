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
// NOTE! This file is #include into main.cpp (no need for another TU for this)
namespace Arcollect {
	namespace sqlite_busy {
		using namespace gui::font;
		static int handler(void* data, int invocation_count)
		{
			if (Arcollect::gui::enabled) {
				// Handle events
				SDL::Event e;
				if (SDL::WaitEvent(e))
					switch (e.type) {
						case SDL_QUIT: {
							// Rethrow this event then give up
							SDL_PeepEvents(&e,1,SDL_ADDEVENT,SDL_QUIT,SDL_QUIT);
						} return false;
						// Wait for Escape
						case SDL_KEYUP: {
							if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
								return false;
						} break;
						default: {
							Arcollect::gui::window_borders::event(e);
						} break;
					}
				// Render text
				// TODO Cache that
				const auto text_elements = Elements::build(FontSize(1.2),
					Align::CENTER,i18n_desktop_app.db_busy_title,U"\n"sv,
					FontSize(1),
					i18n_desktop_app.db_busy_explanation,U"\n"sv,
					SDL::Color{192,192,192,255},i18n_desktop_app.db_busy_hint
				);
				SDL::Point window_size;
				renderer->GetOutputSize(window_size);
				Renderable renderable(text_elements,window_size.x/2);
				// Center
				window_size.x -= renderable.size().x*2;
				window_size.y -= renderable.size().y*2;
				window_size.x /= 2;
				window_size.y /= 2;
				// Blank background
				renderer->SetDrawColor(0,0,0,255);
				renderer->Clear();
				// Draw top bar
				Arcollect::gui::window_borders::render();
				// Draw text
				renderable.render_tl(window_size);
				renderer->Present();
				return true; // Retry
			} else return false; // Don't retry
		}
	}
}
