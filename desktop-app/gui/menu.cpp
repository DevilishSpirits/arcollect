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
#include "menu.hpp"
Arcollect::gui::menu_item::render_context Arcollect::gui::menu::begin_render_context(const Arcollect::gui::modal::render_context &render_ctx)
{
	const SDL::Rect& target = render_ctx.target;
	// Compute menu_size
	SDL::Rect menu_rect{0,0,0,0};
	for (auto& menu: menu_items) {
		SDL::Point size = menu->size();
		menu_rect.h += size.y + 1 + 2*padding.y;
		menu_rect.w  = std::max(menu_rect.w,size.x);
	}
	
	// Compute real menu_rect
	if (anchor_bot && anchor_top) {
		menu_rect.y =                  anchor_distance.y;
		menu_rect.h =  target.h - anchor_distance.y;
	} else if (anchor_bot && !anchor_top)
		menu_rect.y =  target.h - anchor_distance.y - menu_rect.h;
	 else if (!anchor_bot &&  anchor_top)
		menu_rect.y =                  anchor_distance.y;
	else// if (!anchor_bot && !anchor_top) {
		menu_rect.y = (target.h - menu_rect.h)/2;
		
	if (anchor_right && anchor_left) {
		menu_rect.x =                  anchor_distance.x;
		menu_rect.w =  target.w - anchor_distance.x;
	} else if (anchor_right && !anchor_left)
		menu_rect.x =  target.w - anchor_distance.x - menu_rect.w;
	 else if (!anchor_right &&  anchor_left)
		menu_rect.x =                  anchor_distance.x;
	else// if (!anchor_right && !anchor_left) {
		menu_rect.x = (target.w - menu_rect.w)/2;
	
	// Make the menu_item::render_context
	return {
		render_ctx.renderer,
		{menu_rect.x,menu_rect.y,menu_rect.w,-1},
		{menu_rect.x+padding.x,menu_rect.y+padding.y,menu_rect.w-2*padding.x,-1},
		menu_rect,
	};
}
void Arcollect::gui::menu::step_render_context(menu_item::render_context& context, const std::shared_ptr<Arcollect::gui::menu_item> &item)
{
	SDL::Point item_size = item->size();
	// Step size
	context.event_target.y  += context.event_target.h+1;
	context.render_target.y += context.event_target.h+1;
	// Update the context with the item
	context.event_target.h  = item_size.y + 2*padding.y;
	context.render_target.h = item_size.y;
	context.has_focus = item == focused_cell;
}

void Arcollect::gui::menu::render(Arcollect::gui::modal::render_context render_ctx)
{
	menu_item_render_context menu_item_render_ctx = begin_render_context(render_ctx);
	SDL::Rect &render_target = menu_item_render_ctx.render_target;
	SDL::Rect &event_target = menu_item_render_ctx.event_target;
	// Render background
	render_ctx.renderer.SetDrawColor(0,0,0,192);
	render_ctx.renderer.FillRect(menu_item_render_ctx.menu_rect);
	
	// Render cells
	for (auto& menu: menu_items) {
		// Render cell
		step_render_context(menu_item_render_ctx,menu);
		menu->render(menu_item_render_ctx);
		// Render separators
		render_ctx.renderer.SetDrawColor(255,255,255,128);
		render_ctx.renderer.DrawLine(render_target.x, event_target.y - 1, render_target.x + render_target.w, event_target.y - 1);
		// Render focus
		if (menu_item_render_ctx.has_focus) {
			render_ctx.renderer.SetDrawColor(255,255,255,128);
			render_ctx.renderer.FillRect(event_target);
		}
	}
	// Render borders
	render_ctx.renderer.SetDrawColor(255,255,255,255);
	render_ctx.renderer.DrawRect(menu_item_render_ctx.menu_rect);
}

bool Arcollect::gui::menu::event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx)
{
	// Early handlings
	bool propagate = true;
	SDL::Point mouse_focus_event{-1}; // Negative coords = noevent
	switch (e.type) {
		// Reset the ce
		case SDL_MOUSEMOTION: {
			mouse_focus_event = {e.motion.x,e.motion.y};
		} break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: {
			mouse_focus_event = {e.button.x,e.button.y};
		} break;
		// TODO Up/down key docus change
		default:break;
	}
	// Clear focus for now
	if (mouse_focus_event.x > -1)
		focused_cell = NULL;
	
	// Dispatch events to cells
	menu_item_render_context menu_item_render_ctx = begin_render_context(render_ctx);
	for (auto& menu: menu_items) {
		step_render_context(menu_item_render_ctx,menu);
		// Update mouse focus
		if (mouse_focus_event.InRect(menu_item_render_ctx.event_target)) {
			focused_cell = menu;
			menu_item_render_ctx.has_focus = true;
		}
		// Fire event
		propagate &= menu->event(e,menu_item_render_ctx);
	}
	
	return propagate;
}

class popup_menu: public Arcollect::gui::menu {
	public:
		bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override {
			bool result;
			switch (e.type) {
				case SDL_WINDOWEVENT: {
					// Propagate window events
					result = true;
				} break;
				case SDL_QUIT: {
					to_pop = true;
					result = true;
				} break;
				case SDL_MOUSEBUTTONUP: {
					to_pop = true;
					result = false;
				} break;
				default: {
					result = false;
				} break; // This modal grab all events
			}
			result &= Arcollect::gui::menu::event(e,render_ctx);
			return result;
		}
		~popup_menu(void) {
			popup_context_count--;
		}
};

unsigned int Arcollect::gui::menu::popup_context_count = 0;
void Arcollect::gui::menu::popup_context(const std::vector<std::shared_ptr<menu_item>> &menu_items, SDL::Point at, bool anchor_top, bool anchor_left, bool anchor_bot, bool anchor_right)
{
	popup_menu* new_popup_menu = new popup_menu();
	new_popup_menu->anchor_distance = at;
	new_popup_menu->anchor_top = anchor_top;
	new_popup_menu->anchor_left = anchor_left;
	new_popup_menu->anchor_bot = anchor_bot;
	new_popup_menu->anchor_right = anchor_right;
	for (auto& menu_item: menu_items)
		new_popup_menu->append_menu_item(menu_item);
	Arcollect::gui::modal_stack.push_back(std::unique_ptr<modal>(new_popup_menu));
	popup_context_count++;
}

Arcollect::gui::menu_item_label::menu_item_label(const font::Elements& elements) :
	text_line(elements)
{
}
SDL::Point Arcollect::gui::menu_item_label::size(void)
{
	return text_line.size();
}

void Arcollect::gui::menu_item_label::render(const render_context& render_ctx)
{
	const SDL::Rect& target = render_ctx.render_target;
	text_line.render_tl(target.x+(target.w-text_line.size().x)/2,target.y+(target.h-text_line.size().y)/2);
}
bool Arcollect::gui::menu_item_label::event(SDL::Event &e, const render_context& render_ctx)
{
	switch (e.type) {
		case SDL_MOUSEBUTTONDOWN: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			pressed = mouse_pos.InRect(render_ctx.event_target);
		} return true;
		case SDL_MOUSEBUTTONUP: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			if (pressed && mouse_pos.InRect(render_ctx.event_target))
				clicked();
		} return true;
		default: return false;
	}
}
