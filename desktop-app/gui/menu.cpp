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
	// Compute cells
	SDL::Point current_position(topleft);
	current_position.x += padding.x;
	current_position.y += padding.y;
	int max_width = 0;
	const int menu_items_count = static_cast<int>(menu_items.size());
	menu_rects.resize(menu_items_count);
	for (auto i = 0; i < menu_items_count; i++) {
		SDL::Point size = menu_items[i]->size();
		menu_rects[i] = {current_position.x,current_position.y,max_width,size.y};
		max_width = std::max(max_width,size.x);
		current_position.y += size.y + 1 + 2*padding.y;
	}
	// Render background
	current_position.x -= padding.x;
	current_position.y -= padding.y;
	SDL::Rect menu_rect{topleft.x,topleft.y,max_width+2*padding.x,current_position.y-topleft.y};
	renderer->SetDrawColor(0,0,0,192);
	renderer->FillRect(menu_rect);
	renderer->SetDrawColor(255,255,255,255);
	renderer->DrawRect(menu_rect);
	// TODO Render hovering effect
	// Render separators
	renderer->SetDrawColor(128,128,128,255);
	for (auto i = 1; i < menu_items_count; i++) {
		const SDL::Rect &rect = menu_rects[i];
		renderer->DrawLine(rect.x + padding.x, rect.y - padding.y - 1, rect.x + max_width - padding.x, rect.y - padding.y - 1);
	}
	// Render hovered cell
	if (hovered_cell > -1) {
		renderer->SetDrawColor(255,255,255,128);
		menu_rects[hovered_cell].w = max_width;
		SDL::Rect full_rect(menu_rects[hovered_cell]);
		full_rect.x -= padding.x;
		full_rect.y -= padding.y;
		full_rect.w  = max_width + 2 * padding.x;
		full_rect.h += 2 * padding.y;
		renderer->FillRect(full_rect);
	}
	// Render cells
	for (auto i = 0; i < menu_items_count; i++) {
		menu_rects[i].w = max_width;
		menu_items[i]->render(menu_rects[i]);
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
	
	if (broadcast)
		// Pass event to all cells
		for (decltype(menu_items)::size_type i = 0; i < menu_items.size(); i++)
			menu_items[i]->event(e,menu_rects[i]);
	else if (hovered_cell > -1)
		// Pass event to the hovered_cell
		menu_items[hovered_cell]->event(e,menu_rects[hovered_cell]);
	
	return propagate;
}
int Arcollect::gui::menu::get_menu_item_at(SDL::Point cursor)
{
	for (auto i = 0; i < static_cast<int>(menu_items.size()); i++)
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
void Arcollect::gui::menu::popup_context(const std::vector<std::shared_ptr<menu_item>> &menu_items, SDL::Point at)
{
	popup_menu* new_popup_menu = new popup_menu();
	new_popup_menu->topleft = at;
	new_popup_menu->menu_items = menu_items;
	Arcollect::gui::modal_stack.push_back(*new_popup_menu);
	popup_context_count++;
}

Arcollect::gui::menu_item_simple_label::menu_item_simple_label(const char* label, std::function<void()> onclick) :
	text_line(font,label,14),
	text(text_line.render()),
	onclick(onclick)
{
}
SDL::Point Arcollect::gui::menu_item_simple_label::size(void)
{
	SDL::Point point;
	text->QuerySize(point);
	return point;
}

void Arcollect::gui::menu_item_simple_label::render(SDL::Rect target)
{
	// Center the text
	SDL::Point point;
	text->QuerySize(point);
	target.x += (target.w-point.x)/2;
	target.y += (target.h-point.y)/2;
	target.w = point.x;
	target.h = point.y;
	// Render
	renderer->Copy(text.get(),NULL,&target);
}
void Arcollect::gui::menu_item_simple_label::event(SDL::Event &e, SDL::Rect location)
{
	switch (e.type) {
		case SDL_MOUSEBUTTONDOWN: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			pressed = mouse_pos.InRect(location);
		} break;
		case SDL_MOUSEBUTTONUP: {
			SDL::Point mouse_pos{e.button.x,e.button.y};
			if (pressed && mouse_pos.InRect(location))
				onclick();
		} break;
		default: break;
	}
}
