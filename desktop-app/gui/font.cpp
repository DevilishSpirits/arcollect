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
#include "font.hpp"
#include "../sdl2-hpp/SDL.hpp"
#include <SDL_ttf.h>

extern SDL::Renderer *renderer;


Arcollect::gui::TextLine::TextLine(Font &font, const std::string& text, int font_height, int font_style, SDL_Color font_color) :
	font_height(font_height),
	font_color(font_color),
	font_style(font_style),
	text(text),
	font(font)
{
}

SDL::Texture* Arcollect::gui::TextLine::render(void)
{
	if (!cached_render)
		cached_render.reset(SDL::Texture::CreateFromSurface(renderer,font.render_line(font_height,font_color,text.c_str(),font_style).get()));
	return cached_render.get();
}

Arcollect::gui::TextPar::TextPar(Font &font, const std::string& text, int font_height, int font_style, SDL_Color font_color) :
	font_height(font_height),
	font_color(font_color),
	font_style(font_style),
	text(text),
	font(font)
{
}

SDL::Texture* Arcollect::gui::TextPar::render(Uint32 width)
{
	if (!cached_render || (width != cached_width)) {
		cached_render.reset(SDL::Texture::CreateFromSurface(renderer,font.render_paragraph(font_height,font_color,width,text.c_str(),font_style).get()));
		cached_width = width;
	}
	return cached_render.get();
}
