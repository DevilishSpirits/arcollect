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
extern SDL::Renderer *renderer;

void Arcollect::gui::menu::render(void)
{
	SDL::Point screen_size;
	renderer->GetOutputSize(screen_size);
	// Compute menu_size
	SDL::Rect menu_rect{0,0,0,0};
	const int menu_items_count = static_cast<int>(menu_items.size());
	menu_rects.resize(menu_items_count);
	for (auto i = 0; i < menu_items_count; i++) {
		SDL::Point size = menu_items[i]->size();
		menu_rects[i].h = size.y;
		menu_rect.h += size.y + 1 + 2*padding.y;
		menu_rect.w  = std::max(menu_rect.w,size.x);
	}
	menu_rect.w += 2*padding.y;
	
	// Compute real menu_rect
	if (anchor_bot && anchor_top) {
		menu_rect.y =                  anchor_distance.y;
		menu_rect.h =  screen_size.y - anchor_distance.y;
	} else if (anchor_bot && !anchor_top)
		menu_rect.y =  screen_size.y - anchor_distance.y - menu_rect.h;
	 else if (!anchor_bot &&  anchor_top)
		menu_rect.y =                  anchor_distance.y;
	else// if (!anchor_bot && !anchor_top) {
		menu_rect.y = (screen_size.y - menu_rect.h)/2;
		
	if (anchor_right && anchor_left) {
		menu_rect.x =                  anchor_distance.x;
		menu_rect.w =  screen_size.x - anchor_distance.x;
	} else if (anchor_right && !anchor_left)
		menu_rect.x =  screen_size.x - anchor_distance.x - menu_rect.w;
	 else if (!anchor_right &&  anchor_left)
		menu_rect.x =                  anchor_distance.x;
	else// if (!anchor_right && !anchor_left) {
		menu_rect.x = (screen_size.x - menu_rect.w)/2;
	
	// Render background
	renderer->SetDrawColor(0,0,0,192);
	renderer->FillRect(menu_rect);
	renderer->SetDrawColor(255,255,255,255);
	renderer->DrawRect(menu_rect);
	
	// Render cells
	SDL::Rect current_rect{menu_rect.x+padding.x,menu_rect.y+padding.y,menu_rect.w-2*padding.x,0};
	for (auto i = 0; i < menu_items_count; i++) {
		// Compute rect and render
		current_rect.h = menu_rects[i].h;
		menu_rects[i] = current_rect;
		menu_items[i]->render(current_rect);
		// Enlarge rect
		menu_rects[i].x -= padding.x;
		menu_rects[i].y -= padding.y;
		menu_rects[i].w += padding.x*2;
		menu_rects[i].h += padding.y*2;
		// Move current_rect
		current_rect.y += menu_rects[i].h + 1;
	}
	
	// Render separators
	renderer->SetDrawColor(128,128,128,255);
	for (auto i = 1; i < menu_items_count; i++) {
		const SDL::Rect &rect = menu_rects[i];
		renderer->DrawLine(rect.x + padding.x, rect.y - 1, rect.x + rect.w - padding.x, rect.y - 1);
	}
	
	// Render hovered cell
	if (hovered_cell > -1) {
		renderer->SetDrawColor(255,255,255,128);
		renderer->FillRect(menu_rects[hovered_cell]);
	}
}

bool Arcollect::gui::menu::event(SDL::Event &e)
{
	bool propagate; // Propagate event to other modals
	bool broadcast = false; // Broadcast event to all items
	switch (e.type) {
		case SDL_MOUSEMOTION: {
			hovered_cell = get_menu_item_at({e.motion.x,e.motion.y});
			propagate = false;
		} break;
		case SDL_MOUSEBUTTONUP: {
			propagate = false;
			broadcast = true;
		} break;
		default: propagate = true;
	}
	
	if (broadcast) {
		// Pass event to all cells
		// Note: menu_rects[i].x and menu_rects[i].w are the same everywhere
		SDL::Rect render_location{menu_rects[0].x + padding.x,menu_rects[0].y,menu_rects[0].w - 2*padding.x,0};
		for (decltype(menu_items)::size_type i = 0; i < menu_items.size(); i++) {
			render_location.h = menu_rects[i].h - 2*padding.x;
			menu_items[i]->event(e,menu_rects[i],render_location);
			render_location.y += menu_rects[i].h;
		}
	} else if (hovered_cell > -1)
		// Pass event to the hovered_cell
		menu_items[hovered_cell]->event(e,menu_rects[hovered_cell],{
			menu_rects[hovered_cell].x + padding.x,
			menu_rects[hovered_cell].y + padding.y,
			menu_rects[hovered_cell].w - 2*padding.x,
			menu_rects[hovered_cell].h - 2*padding.y,
		});
	
	return propagate;
}
int Arcollect::gui::menu::get_menu_item_at(SDL::Point cursor)
{
	for (auto i = 0; i < static_cast<int>(menu_rects.size()); i++)
		if (cursor.InRect(menu_rects[i]))
			return i;
	// No match found
	return -1;
}

class popup_menu: public Arcollect::gui::menu {
	public:
		bool event(SDL::Event &e) override {
			if (e.type == SDL_MOUSEBUTTONUP)
				Arcollect::gui::modal_stack.pop_back();
			Arcollect::gui::menu::event(e);
			switch (e.type) {
				case SDL_WINDOWEVENT: {
					// Propagate window events
				} return true;
				case SDL_QUIT: {
					delete this;
				} return true;
				case SDL_MOUSEBUTTONUP: {
					delete this;
				} return false;
				default: {
				} return false; // This modal grab all events
			}
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
	new_popup_menu->menu_items = menu_items;
	Arcollect::gui::modal_stack.push_back(*new_popup_menu);
	popup_context_count++;
}

Arcollect::gui::menu_item_simple_label::menu_item_simple_label(const char* label, std::function<void()> onclick) :
	text_line(label,14),
	onclick(onclick)
{
}
SDL::Point Arcollect::gui::menu_item_simple_label::size(void)
{
	return text_line.size();
}

void Arcollect::gui::menu_item_simple_label::render(SDL::Rect target)
{
	text_line.render_tl(target.x+(target.w-text_line.size().x)/2,target.y+(target.h-text_line.size().y)/2);
}
void Arcollect::gui::menu_item_simple_label::event(SDL::Event &e, const SDL::Rect &event_location, const SDL::Rect &render_location)
{
	switch (e.type) {
		case SDL_MOUSEBUTTONDOWN: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			pressed = mouse_pos.InRect(event_location);
		} break;
		case SDL_MOUSEBUTTONUP: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			if (pressed && mouse_pos.InRect(event_location))
				onclick();
		} break;
		default: break;
	}
}
