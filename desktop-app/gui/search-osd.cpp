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
#include "font.hpp"
#include "search-osd.hpp"
#include "slideshow.hpp"
#include "window-borders.hpp"

Arcollect::gui::search_osd Arcollect::gui::search_osd_modal;

bool Arcollect::gui::search_osd::event(SDL::Event &e)
{
	switch (e.type) {
		case SDL_WINDOWEVENT: {
		} return true;
		case SDL_KEYDOWN: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_BACKSPACE: {
					if (!text.empty())
						text.pop_back();
					update_background(text,true);
				} break;
				default: {
				} break;
			}
		} return false;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_ESCAPE: {
					update_background(saved_text,true);
				} //falltrough;
				case SDL_SCANCODE_RETURN: {
					SDL_StopTextInput();
					modal_stack.pop_back();
				} break;
				default: {
				} break;
			}
		} return false;
		case SDL_TEXTINPUT: {
			text += std::string(e.text.text);
			update_background(text,true);
		} return false;
		default: {
		} return false;
	}
}
void Arcollect::gui::search_osd::render(void)
{
	SDL::Point screen_size;
	renderer->GetOutputSize(screen_size);
	render_titlebar({0,0,screen_size.x,Arcollect::gui::window_borders::title_height},screen_size.x);
}
void Arcollect::gui::search_osd::render_titlebar(SDL::Rect target, int window_width)
{
	// Render text
	Arcollect::gui::Font font;
	const int title_border = target.h/4;
	const int font_height = target.h-2*title_border;
	Arcollect::gui::TextLine search_line(font,text.c_str(),font_height);
	SDL::Texture *text = search_line.render();
	// Compute sizes
	SDL::Point text_size;
	text->QuerySize(text_size);
	// Blit
	SDL::Rect rect{target.h+title_border,title_border,text_size.x,text_size.y};
	renderer->Copy(text,NULL,&rect);
}
void Arcollect::gui::search_osd::push(void)
{
	text = saved_text = Arcollect::gui::get_current_search();
	SDL_StartTextInput();
	if (&modal_stack.back().get() != &Arcollect::gui::background_vgrid)
		modal_stack.push_back(Arcollect::gui::background_vgrid); // Force display of background_vgrid
	modal_stack.push_back(*this);
}
