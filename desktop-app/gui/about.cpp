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
#include "about.hpp"
#include "menu.hpp"
#include <config.h>

extern SDL::Renderer *renderer;
static bool about_window_modal_shown = false;
void Arcollect::gui::about_window::show(void) 
{
	if (!about_window_modal_shown) {
		Arcollect::gui::modal_stack.push_back(Arcollect::gui::about_window_modal);
		about_window_modal_shown = true;
	}
}

Arcollect::gui::about_window Arcollect::gui::about_window_modal;
bool Arcollect::gui::about_window::event(SDL::Event &e) {
	switch (e.type) {
		case SDL_MOUSEBUTTONUP:
		case SDL_KEYUP: {
			Arcollect::gui::modal_stack.pop_back();
			about_window_modal_shown = false;
		} break;
	}
	return true;
}
void Arcollect::gui::about_window::render()
{
	// Render title
	SDL::Point window_size;
	renderer->GetOutputSize(window_size);
	
	// About text
	std::unique_ptr<Arcollect::gui::font::Renderable> cached_renderable;
	if ((cache_window_width != window_size.x)||!render_cache) {
		using namespace Arcollect::gui::font;
		Elements elements;
		elements << Align::CENTER
			<< FontSize(48) << U"Arcollect "sv ARCOLLECT_VERSION_STR "\n\n"
			<< FontSize(22) << U"Your personal visual artwork library\n"sv
			<< FontSize(16) << U""sv ARCOLLECT_WEBSITE_STR "\n"
			<< U"Powered by:\n"sv
			   CPP_STD_STR " ⸱ " CXX_COMPILER_TITLE_STR " ⸱ SDL2 ⸱ SQLite3 ⸱ OpenImageIO ⸱ LittleCMS ⸱ FreeType2 ⸱ HarfBuzz ⸱ Many other…\n"
			   "\n"
			   "Arcollect is licensed under the GNU General Public License version 3 (GPL-3) or later.\n"
			   "It use and bundle third-party dependencies released under another licenses\n"
			   "Links to projects websites are available in the top bar menu.\n"
		;
		render_cache = std::make_unique<Arcollect::gui::font::Renderable>(elements,window_size.x-window_size.x/10);
		cache_window_width = window_size.x;
	}
	SDL::Point welcome_text_dst{window_size.x/20,(window_size.y-render_cache->size().y)/2};
	renderer->SetDrawColor(0,0,0,128);
	renderer->FillRect();
	render_cache->render_tl(welcome_text_dst);
}

static void open_github(void) {
	SDL_OpenURL(ARCOLLECT_WEBSITE_STR);
}
static void open_gpl3(void) {
	SDL_OpenURL("https://www.gnu.org/licenses/gpl-3.0.html");
}
static void open_sdl2(void) {
	SDL_OpenURL("https://www.libsdl.org/");
}
static void open_oiio(void) {
	SDL_OpenURL("https://openimageio.org/");
}
static void open_lcms2(void) {
	SDL_OpenURL("https://www.littlecms.com/");
}
static void open_freetype2(void) {
	SDL_OpenURL("https://www.freetype.org/");
}
static void open_harfbuzz(void) {
	SDL_OpenURL("https://harfbuzz.github.io/");
}

std::vector<std::shared_ptr<Arcollect::gui::menu_item>> Arcollect::gui::about_window::top_menu(void)
{
	return {
		std::make_shared<Arcollect::gui::menu_item_simple_label>(U"Arcollect"s,open_github),
		std::make_shared<Arcollect::gui::menu_item_simple_label>(U"GPL-3 license"s,open_gpl3),
		std::make_shared<Arcollect::gui::menu_item_simple_label>(U"SDL2"s,open_sdl2),
		std::make_shared<Arcollect::gui::menu_item_simple_label>(U"OpenImageIO"s,open_oiio),
		std::make_shared<Arcollect::gui::menu_item_simple_label>(U"LittleCMS"s,open_lcms2),
		std::make_shared<Arcollect::gui::menu_item_simple_label>(U"FreeType2"s,open_freetype2),
		std::make_shared<Arcollect::gui::menu_item_simple_label>(U"HarfBuzz"s,open_harfbuzz),
		
	};
};
