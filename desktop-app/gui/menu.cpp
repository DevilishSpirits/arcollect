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

void Arcollect::gui::menu::render(Arcollect::gui::modal::render_context render_ctx)
{
	auto& target = render_ctx.target;
	// Compute menu_size
	SDL::Rect menu_rect{0,0,0,0};
	for (auto& menu_pair: menu_items) {
		SDL::Point size = menu_pair.first->size();
		menu_pair.second.h = size.y;
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
	
	// Render background
	renderer->SetDrawColor(0,0,0,192);
	renderer->FillRect(menu_rect);
	renderer->SetDrawColor(255,255,255,255);
	renderer->DrawRect(menu_rect);
	
	// Render cells
	SDL::Rect current_rect{menu_rect.x+padding.x,menu_rect.y+padding.y,menu_rect.w-2*padding.x,0};
	for (auto& menu_pair: menu_items) {
		SDL::Rect &rect = menu_pair.second;
		// Compute rect and render
		current_rect.h = rect.h;
		rect = current_rect;
		menu_pair.first->render(current_rect);
		// Enlarge rect
		rect.x -= padding.x;
		rect.y -= padding.y;
		rect.w += padding.x*2;
		rect.h += padding.y*2;
		// Move current_rect
		current_rect.y += rect.h + 1;
	}
	
	// Render separators
	renderer->SetDrawColor(128,128,128,255);
	for (auto& menu_pair: menu_items) {
		const SDL::Rect &rect = menu_pair.second;
		renderer->DrawLine(rect.x + padding.x, rect.y - 1, rect.x + rect.w - padding.x, rect.y - 1);
	}
	
	// Render hovered cell
	if (hovered_cell > -1) {
		renderer->SetDrawColor(255,255,255,128);
		renderer->FillRect(menu_items[hovered_cell].second);
	}
}

bool Arcollect::gui::menu::event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx)
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
		const auto& menu_rect0 = menu_items[0].second;
		SDL::Rect render_location{menu_rect0.x + padding.x,menu_rect0.y,menu_rect0.w - 2*padding.x,0};
		for (auto& menu_pair: menu_items) {
			const SDL::Rect &rect =  menu_pair.second;
			render_location.h = rect.h - 2*padding.x;
			menu_pair.first->event(e,rect,render_location);
			render_location.y +=  rect.h;
		}
	} else if (hovered_cell > -1) {
		const SDL::Rect &hovered_cell_rect = menu_items[hovered_cell].second;
		// Pass event to the hovered_cell
		menu_items[hovered_cell].first->event(e,hovered_cell_rect,{
			hovered_cell_rect.x + padding.x,
			hovered_cell_rect.y + padding.y,
			hovered_cell_rect.w - 2*padding.x,
			hovered_cell_rect.h - 2*padding.y,
		});
	}
	
	return propagate;
}
int Arcollect::gui::menu::get_menu_item_at(SDL::Point cursor)
{
	for (int i = 0; i < static_cast<int>(menu_items.size()); i++)
		if (cursor.InRect(menu_items[i].second))
			return i;
	// No match found
	return -1;
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

void Arcollect::gui::menu_item_label::render(SDL::Rect target)
{
	text_line.render_tl(target.x+(target.w-text_line.size().x)/2,target.y+(target.h-text_line.size().y)/2);
}
void Arcollect::gui::menu_item_label::event(SDL::Event &e, const SDL::Rect &event_location, const SDL::Rect &render_location)
{
	switch (e.type) {
		case SDL_MOUSEBUTTONDOWN: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			pressed = mouse_pos.InRect(event_location);
		} break;
		case SDL_MOUSEBUTTONUP: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			if (pressed && mouse_pos.InRect(event_location))
				clicked();
		} break;
		default: break;
	}
}
