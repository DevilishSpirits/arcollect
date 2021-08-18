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
#include "first-run.hpp"
#include "../config.hpp"

extern SDL::Renderer *renderer;

Arcollect::gui::first_run Arcollect::gui::first_run_modal;
bool Arcollect::gui::first_run::event(SDL::Event &e) {
	switch (e.type) {
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_RIGHT: {
					Arcollect::gui::modal_stack.pop_back();
					Arcollect::config::first_run = true;
					render_cache.reset(); // Free some resources
				} return true;
				default:return false;
			}
		} break;
	}
	return true;
}

void Arcollect::gui::first_run::render()
{
	// Render title
	SDL::Point window_size;
	renderer->GetOutputSize(window_size);
	
	// Welcome text
	std::unique_ptr<Arcollect::gui::font::Renderable> cached_renderable;
	if ((cache_window_width != window_size.x)||!render_cache) {
		render_cache = std::make_unique<Arcollect::gui::font::Renderable>(Arcollect::gui::font::Elements(U"This is your first Arcollect run !\n\n"
		"One day. I discovered that I love visual artworks and I made Arcollect to organize my growing collection. It allows you to easily save pictures you find on the internet in a few click and save a bunch metadata like who did that and where you took the picture.\n\n"
		"With the associated web extension, buttons will appear on DeviantArt, e621 and FurAffinity artworks pages to save them in your personal collection.\n\n"
		"Arcollect is a free and open-source software. It respect your privacy and will never judge you. See " ARCOLLECT_WEBSITE_STR " to learn more.\n\n"
		"Now press and release the right arrow to see what's next..."s,22),window_size.x-window_size.x/10);
		cache_window_width = window_size.x;
	}
	const auto welcome_text_boxrect_padding = window_size.x/40;
	SDL::Point welcome_text_dst{window_size.x/20,(window_size.y-render_cache->size().y)/2};
	SDL::Rect welcome_text_boxrect{welcome_text_dst.x-welcome_text_boxrect_padding,welcome_text_dst.y-welcome_text_boxrect_padding,render_cache->size().x+2*welcome_text_boxrect_padding,render_cache->size().y+2*welcome_text_boxrect_padding};
	renderer->SetDrawColor(0,0,0,224);
	renderer->FillRect(welcome_text_boxrect);
	render_cache->render_tl(welcome_text_dst);
}
