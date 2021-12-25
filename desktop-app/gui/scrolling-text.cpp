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
#include "../config.hpp"
#include "scrolling-text.hpp"
extern SDL::Renderer *renderer;
void Arcollect::gui::scrolling_text::scroll_text(int line_delta, const SDL::Rect &rect)
{
	if (renderable) {
		int border = rect.w/10;
		auto target = scroll.val_target;
		target += line_delta*Arcollect::config::writing_font_size;
		if (target > renderable->size().y - rect.h + border + border)
			target = renderable->size().y - rect.h + border + border;
		else if (target < 0)
			target = 0;
		scroll = target;
	}
}
void Arcollect::gui::scrolling_text::set_static_elements(const Arcollect::gui::font::Elements& new_elements)
{
	elements = new_elements;
	renderable.reset();
}
void Arcollect::gui::scrolling_text::render(SDL::Rect target)
{
	SDL::Rect progress_bar{target.x,target.y};
	int current_scroll = scroll;
	int border = target.w/10;
	target.x += border;
	target.y += border;
	// Shape text if not made already
	if (!renderable || (target.w != renderable_target_width)) {
		Arcollect::gui::font::RenderConfig render_config;
		render_config.base_font_height = Arcollect::config::writing_font_size;
		render_config.always_justify = true;
		renderable = std::make_unique<Arcollect::gui::font::Renderable>(elements,target.w-border-border,render_config);
		renderable_target_width = target.w;
	}
	// Render text
	int max_scroll = renderable->size().y - target.h + border + border;
	renderable->render_tl(target.x,target.y-current_scroll);
	// Render progress bar
	progress_bar.w = target.w;
	progress_bar.h = Arcollect::config::writing_font_size/8;
	progress_bar.y += target.h-progress_bar.h;
	renderer->SetDrawColor(0,0,0,192);
	renderer->FillRect(progress_bar);
	progress_bar.w = current_scroll*target.w/max_scroll;
	renderer->SetDrawColor(255,255,255,192);
	renderer->FillRect(progress_bar);
}
bool Arcollect::gui::scrolling_text::event(SDL::Event &e, SDL::Rect target)
{
	const auto text_scroll_speed = 5;
	switch (e.type) {
		case SDL_KEYDOWN: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_UP: {
					scroll_text(-text_scroll_speed,target);
				} return false;
				case SDL_SCANCODE_DOWN: {
					scroll_text(+text_scroll_speed,target);
				} return false;
				default:break;
			}
		} break;
		case SDL_MOUSEWHEEL: {
			scroll_text(-e.wheel.y*text_scroll_speed,target);
		} return false;
		case SDL_MOUSEMOTION: {
			if (e.motion.state & SDL_BUTTON(1)) {
				// Scroll text on left-click drag
				scroll.val_target -= e.motion.yrel;
				scroll_text(0,target);
			}
		} return false;
	}
	return true;
}
