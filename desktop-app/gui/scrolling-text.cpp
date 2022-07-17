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
// From https://en.cppreference.com/w/cpp/utility/variant/visit
template<class> inline constexpr bool always_false_v = false;
bool Arcollect::gui::scrolling_text::elements_available(void) const
{
	if (std::holds_alternative<Arcollect::gui::font::Elements>(data))
		return true;
	else if (std::holds_alternative<std::shared_ptr<Arcollect::db::download>>(data))
		return std::get<std::shared_ptr<Arcollect::db::download>>(data)->queue_for_load();
	else return false; // FIXME Make this case an error at compile time
}
Arcollect::gui::font::Elements& Arcollect::gui::scrolling_text::get_elements(void)
{
	if (std::holds_alternative<Arcollect::gui::font::Elements>(data))
		return std::get<Arcollect::gui::font::Elements>(data);
	else if (std::holds_alternative<std::shared_ptr<Arcollect::db::download>>(data))
		return *std::get<std::shared_ptr<Arcollect::db::download>>(data)->query_data<Arcollect::gui::font::Elements>();
	else {// FIXME Make this case an error at compile time
		static Arcollect::gui::font::Elements wtf;
		return wtf;
	}
}
void Arcollect::gui::scrolling_text::render(Arcollect::gui::modal::render_context render_ctx)
{
	auto &target = render_ctx.target;
	if (!elements_available())
		return;
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
		renderable = std::make_unique<Arcollect::gui::font::Renderable>(get_elements(),target.w-border-border,render_config);
		renderable_target_width = target.w;
	}
	// Render text
	int max_scroll = renderable->size().y - target.h + border + border;
	renderable->render_tl(target.x,target.y-current_scroll);
	// Render progress bar
	progress_bar.w = target.w;
	progress_bar.h = Arcollect::config::writing_font_size/8;
	progress_bar.y += target.h-progress_bar.h;
	render_ctx.renderer.SetDrawColor(0,0,0,192);
	render_ctx.renderer.FillRect(progress_bar);
	progress_bar.w = current_scroll*target.w/max_scroll;
	render_ctx.renderer.SetDrawColor(255,255,255,192);
	render_ctx.renderer.FillRect(progress_bar);
}
bool Arcollect::gui::scrolling_text::event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx)
{
	const auto text_scroll_speed = 5;
	switch (e.type) {
		case SDL_KEYDOWN: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_UP: {
					scroll_text(-text_scroll_speed,render_ctx.target);
				} return false;
				case SDL_SCANCODE_DOWN: {
					scroll_text(+text_scroll_speed,render_ctx.target);
				} return false;
				case SDL_SCANCODE_PAGEUP: {
					scroll_text(-text_scroll_speed*4,render_ctx.target);
				} return false;
				case SDL_SCANCODE_PAGEDOWN: {
					scroll_text(+text_scroll_speed*4,render_ctx.target);
				} return false;
				default:break;
			}
		} break;
		case SDL_MOUSEWHEEL: {
			scroll_text(-e.wheel.y*text_scroll_speed,render_ctx.target);
		} return false;
		case SDL_MOUSEMOTION: {
			if (e.motion.state & SDL_BUTTON(1)) {
				// Scroll text on left-click drag
				scroll.val_target -= e.motion.yrel;
				scroll_text(0,render_ctx.target);
			}
		} return false;
	}
	return true;
}
