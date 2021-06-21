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
#include "font.hpp"
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
	
	Arcollect::gui::Font font;
	// Welcome text
	Arcollect::gui::TextPar welcome_text(font,
	"This is your first Arcollect run !\n\n"
	"One day. I discovered that I love visual artworks and I made Arcollect to organize my growing collection. It allows you to easily save pictures you find on the internet in a few click and save a bunch metadata like who did that and where you took the picture.\n\n"
	"With the associated web extension, buttons will appear on DeviantArt, e621 and FurAffinity artworks pages to save them in your personnal collection.\n\n"
	"Arcollect is a free and open-source software. It respect your privacy and will never judge you. See " ARCOLLECT_WEBSITE_STR " to learn more.\n\n"
	"Now press and release the right arrow to see what's next..."
	,16);
	SDL::Texture* welcome_text_text(welcome_text.render(window_size.x-window_size.x/10));
	SDL::Point welcome_text_size;
	welcome_text_text->QuerySize(welcome_text_size);
	SDL::Rect welcome_text_dstrect{window_size.x/20,(window_size.y-welcome_text_size.y)/2,welcome_text_size.x,welcome_text_size.y};
	const auto welcome_text_boxrect_padding = window_size.x/40;
	SDL::Rect welcome_text_boxrect{welcome_text_dstrect.x-welcome_text_boxrect_padding,welcome_text_dstrect.y-welcome_text_boxrect_padding,welcome_text_dstrect.w+2*welcome_text_boxrect_padding,welcome_text_dstrect.h+2*welcome_text_boxrect_padding};
	renderer->SetDrawColor(0,0,0,224);
	renderer->FillRect(welcome_text_boxrect);
	renderer->Copy(welcome_text_text,NULL,&welcome_text_dstrect);
}
void Arcollect::gui::first_run::render_titlebar(SDL::Rect target, int window_width)
{
	// Render icon
	SDL::Rect icon_rect{target.x,target.y,target.h,target.h};
	// TODO renderer->Copy(arcollect_icon,NULL,&icon_rect);
	// Render title
	const int title_border = target.h/4;
	Arcollect::gui::Font font;
	Arcollect::gui::TextLine title_line(font,"Arcollect " ARCOLLECT_VERSION_STR,target.h-2*title_border);
	SDL::Texture* title_line_text(title_line.render());
	SDL::Point title_line_size;
	title_line_text->QuerySize(title_line_size);
	SDL::Rect title_line_dstrect{target.x+title_border+target.h,target.y+title_border,title_line_size.x,title_line_size.y};
	renderer->Copy(title_line_text,NULL,&title_line_dstrect);
}
