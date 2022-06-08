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
#include "window-borders.hpp"
#include "about.hpp"
#include "menu.hpp"
#include "rating-selector.hpp"
#include "../i18n.hpp"
static bool display_bar = false;
int Arcollect::gui::window_borders::title_height = 32;
const int Arcollect::gui::window_borders::resize_width = 4;
extern SDL_Window    *window;
extern SDL::Renderer *renderer;


static int &title_button_height  = Arcollect::gui::window_borders::title_height;
static int &title_button_width   = Arcollect::gui::window_borders::title_height;
enum TitleButton: int {
	TITLEBTN_CLOSE,
	TITLEBTN_MAXIMIZE,
	TITLEBTN_FULLSCREEN,
	TITLEBTN_MINIMIZE,
	TITLEBTN_MENU,
	TITLEBTN_N,
	TITLEBTN_NONE = TITLEBTN_N,
};
static TitleButton titlebtn_hovered = TITLEBTN_NONE;
static TitleButton titlebtn_pressed = TITLEBTN_NONE;

static std::vector<std::shared_ptr<Arcollect::gui::menu_item>> topbar_menu_items;

static SDL_HitTestResult hit_test(SDL_Window *, const SDL_Point *point, void* data)
{
	auto window_flags = SDL_GetWindowFlags(window);
	SDL::Point window_size;
	renderer->GetOutputSize(window_size);
	// TODO Check errors
	// Ignore borders if in fullscreen mode
	if (((window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != SDL_WINDOW_FULLSCREEN_DESKTOP)&&((window_flags & SDL_WINDOW_MAXIMIZED) != SDL_WINDOW_MAXIMIZED)) {
		// Compute resize boxes
		const bool l_resize = point->x <= Arcollect::gui::window_borders::resize_width;
		const bool t_resize = point->y <= Arcollect::gui::window_borders::resize_width;
		const bool r_resize = window_size.x - point->x <= Arcollect::gui::window_borders::resize_width;
		const bool b_resize = window_size.y - point->y <= Arcollect::gui::window_borders::resize_width;
		// Check for corner resizes
		if (l_resize && t_resize) return SDL_HITTEST_RESIZE_TOPLEFT;
		if (t_resize && r_resize) return SDL_HITTEST_RESIZE_TOPRIGHT;
		if (r_resize && b_resize) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
		if (b_resize && l_resize) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
		// Check for border resizes
		if (l_resize) return SDL_HITTEST_RESIZE_LEFT;
		if (t_resize) return SDL_HITTEST_RESIZE_TOP;
		if (r_resize) return SDL_HITTEST_RESIZE_RIGHT;
		if (b_resize) return SDL_HITTEST_RESIZE_BOTTOM;
	}
	// Check for dragable area
	if ((point->y <= Arcollect::gui::window_borders::title_height)&&
	 (point->x < window_size.x-(title_button_width*TITLEBTN_N)))
		return SDL_HITTEST_DRAGGABLE;
	
	// No match
	return SDL_HITTEST_NORMAL;
}

void Arcollect::gui::window_borders::init(SDL_Window *window)
{
	// Set title_button_height
	SDL_Rect screen0_rect;
	if (!SDL_GetDisplayBounds(0,&screen0_rect))
		title_height = std::max(title_height,screen0_rect.h/32);
	// Init menus
	topbar_menu_items.emplace_back(std::make_shared<Arcollect::gui::rating_selector_menu>());
	topbar_menu_items.emplace_back(std::make_shared<Arcollect::gui::menu_item_simple_label>(i18n_desktop_app.about_arcollect,Arcollect::gui::about_window::show));
	// Init borders
	SDL_SetWindowHitTest(window,hit_test,NULL);
	SDL_SetWindowBordered(window,SDL_FALSE);
	SDL_SetWindowMinimumSize(window,title_button_width*TITLEBTN_N,title_button_height);
}

// Return if the menu has been popped up
static bool popup_title_context_menu(void)
{
	if (Arcollect::gui::menu::popup_context_count == 0) {
		// Pop menu
		std::vector<std::shared_ptr<Arcollect::gui::menu_item>> menu = Arcollect::gui::modal_back().top_menu();
		for (auto& item: topbar_menu_items)
			menu.emplace_back(item);
		Arcollect::gui::menu::popup_context(menu,{title_button_width,Arcollect::gui::window_borders::title_height},true,false,false,true);
		return true;
	} else return false;
}

bool Arcollect::gui::window_borders::event(SDL::Event &e)
{
	// TODO Proper cursor allocations
	// TODO Use X11 full cursor range
	SDL::Point cursor_position{e.motion.x,e.motion.y};
	// Get window size
	auto window_flags = SDL_GetWindowFlags(window);
	SDL::Point window_size;
	renderer->GetOutputSize(window_size);
	switch (e.type) {
		case SDL_QUIT: {
			// Destroy menus GUI resources
			topbar_menu_items.clear();
		} return true;
		case SDL_WINDOWEVENT: {
			switch (e.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				case SDL_WINDOWEVENT_RESIZED: {
					// Falltrough SDL_MOUSEMOTION code with updated mouse data
					SDL_GetMouseState(&cursor_position.x,&cursor_position.y);
				} break;
				default: {
					// Do nothing
				} return true;
			}
		}
		case SDL_MOUSEMOTION: {
			static SDL_Cursor* cursor_normal = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
			static SDL_Cursor* cursor_nwse   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
			static SDL_Cursor* cursor_nesw   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
			static SDL_Cursor* cursor_we     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
			static SDL_Cursor* cursor_ns     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
			SDL_HitTestResult test_result = hit_test(NULL,(SDL_Point*)&cursor_position,NULL);
			// Check if we display the bar
			display_bar = cursor_position.y <= Arcollect::gui::window_borders::title_height;
			// Check which title button is hovered
			if (display_bar) {
				// Compute position
				int titlebtn_position = (window_size.x - cursor_position.x)/title_button_width;
				if (titlebtn_position < TITLEBTN_N)
					titlebtn_hovered = static_cast<TitleButton>(titlebtn_position);
				else titlebtn_hovered = TITLEBTN_NONE;
			} else titlebtn_hovered = TITLEBTN_NONE;
			// Update cursor
			switch (test_result) {
				case SDL_HITTEST_RESIZE_TOPLEFT:
				case SDL_HITTEST_RESIZE_BOTTOMRIGHT: {
					SDL_SetCursor(cursor_nwse);
				} break;
				case SDL_HITTEST_RESIZE_TOP:
				case SDL_HITTEST_RESIZE_BOTTOM: {
					SDL_SetCursor(cursor_ns);
				} break;
				case SDL_HITTEST_RESIZE_TOPRIGHT:
				case SDL_HITTEST_RESIZE_BOTTOMLEFT: {
					SDL_SetCursor(cursor_nesw);
				} break;
				case SDL_HITTEST_RESIZE_LEFT:
				case SDL_HITTEST_RESIZE_RIGHT: {
					SDL_SetCursor(cursor_we);
				} break;
				case SDL_HITTEST_DRAGGABLE: {
					SDL_SetCursor(cursor_normal);
				} break;
				case SDL_HITTEST_NORMAL:
				default: {
					SDL_SetCursor(cursor_normal);
				} return true;
			}
		} return e.type == SDL_WINDOWEVENT; // Propage window resize events
		case SDL_MOUSEBUTTONDOWN: {
			if (titlebtn_hovered != TITLEBTN_NONE)
				titlebtn_pressed = titlebtn_hovered;
		} return cursor_position.y >= Arcollect::gui::window_borders::title_height;
		case SDL_MOUSEBUTTONUP: {
			bool button_clicked = titlebtn_pressed == titlebtn_hovered;
			titlebtn_pressed = TITLEBTN_NONE;
			if (button_clicked) {
				switch (titlebtn_hovered) {
					case TITLEBTN_CLOSE: {
						// Generate a quit event
						SDL_Event event;
						event.type = SDL_QUIT;
						SDL_PushEvent(&event);
					} break;
					case TITLEBTN_MAXIMIZE: {
						// Maximize or restore window
						if (window_flags & SDL_WINDOW_MAXIMIZED)
							SDL_RestoreWindow(window);
						else {
							if (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
								set_fullscreen(false);
							SDL_MaximizeWindow(window);
						}
					} break;
					case TITLEBTN_MINIMIZE: {
						// Minimize window
						SDL_MinimizeWindow(window);
					} break;
					case TITLEBTN_FULLSCREEN: {
						// Toggle fullscreen
						if (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
							set_fullscreen(false);
						else set_fullscreen(true);
					} break;
					case TITLEBTN_MENU: {
						// Don't pop if there is a context menu
						if (!popup_title_context_menu())
							/* Hide the menu
							 *
							 * Context menu popdown upon SDL_MOUSEBUTTONUP. By default we
							 * don't propagate this event for clicks on the menu bar. But to
							 * hide the menu, we make a special exception
							 */
							return true;
					} break;
					// Suppress warnings about missing TITLEBTN_NONE
					case TITLEBTN_NONE: break;
				}
			}
		} return cursor_position.y >= Arcollect::gui::window_borders::title_height;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_F10: {
					// Title button menu
					// Don't pop if there is a context menu
					if (popup_title_context_menu())
						return false;
					else {
						// We mutate e into a click on TITLEBTN_MENU to pop the menu.
						e.type = SDL_MOUSEBUTTONUP;
						e.button.x = window_size.x;
						e.button.y = Arcollect::gui::window_borders::title_height/2;
						return true;
					}
				}
				case SDL_SCANCODE_F11: {
					// Toggle fullscreen
					if (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
						set_fullscreen(false);
					else set_fullscreen(true);
				} return false;
				default: break;
			}
		} return true;
	}
	return true;
}
void Arcollect::gui::window_borders::render(Arcollect::gui::modal::render_context &render_ctx)
{
	const int title_button_padding = Arcollect::gui::window_borders::title_height/3;
	if (display_bar) {
		render_ctx.renderer.SetDrawBlendMode(SDL::BLENDMODE_BLEND);
		// Get bar size and cursor position
		SDL::Point cursor_position;
		SDL_GetMouseState(&cursor_position.x,&cursor_position.y);
		// Draw modal title
		SDL::Rect modal_bar{0,0,render_ctx.target.w-TITLEBTN_N*title_button_width,Arcollect::gui::window_borders::title_height};
		render_ctx.renderer.SetDrawColor(0,0,0,128);
		render_ctx.renderer.FillRect(modal_bar);
		modal_back().render_titlebar(render_ctx);
		// Draw buttons background over title if it overflow
		SDL::Rect title_button_area{modal_bar.w,0,TITLEBTN_N*title_button_width,Arcollect::gui::window_borders::title_height};
		render_ctx.renderer.SetDrawColor(0,0,0,128);
		render_ctx.renderer.FillRect(title_button_area);
		// Compute buttons rects
		SDL::Rect btn_rect = {
			render_ctx.target.w - title_button_width,0,
			title_button_width,title_button_height
		};
		SDL::Rect btn_inner_rect = {
			btn_rect.x + title_button_padding,
			btn_rect.y + title_button_padding,
			btn_rect.w - 2*title_button_padding,
			btn_rect.h - 2*title_button_padding,
		};
		const int btn_ymid = btn_rect.y + (btn_rect.h)/2;
		const int btn_inner_ybot = btn_inner_rect.y + btn_inner_rect.h;
		render_ctx.renderer.SetDrawColor(255,255,255,192);
		// Draw close button (a cross)
		SDL::Point close_tl{btn_inner_rect.x, btn_inner_rect.y};
		SDL::Point close_br{btn_inner_rect.x + btn_inner_rect.w, btn_inner_rect.y + btn_inner_rect.h};
		render_ctx.renderer.DrawLine(close_tl,close_br);
		render_ctx.renderer.DrawLine(close_tl.x,close_br.y,close_br.x,close_tl.y);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Draw maximize button (a square)
		render_ctx.renderer.DrawRect(btn_inner_rect);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Draw fullscreen button (a filled square)
		SDL::Rect fullscreen_rect = btn_inner_rect;
		render_ctx.renderer.DrawRect(fullscreen_rect);
		fullscreen_rect.x += 2;
		fullscreen_rect.y += 2;
		fullscreen_rect.w -= 4;
		fullscreen_rect.h -= 4;
		render_ctx.renderer.FillRect(fullscreen_rect);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Draw minimize button (a square)
		render_ctx.renderer.DrawLine(btn_inner_rect.x,btn_inner_ybot,btn_inner_rect.x+btn_inner_rect.w,btn_inner_ybot);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Draw menu button (triangle)
		render_ctx.renderer.DrawLine(btn_inner_rect.x,btn_ymid,btn_inner_rect.x+btn_inner_rect.w,btn_ymid);
		render_ctx.renderer.DrawLine(btn_inner_rect.x,btn_ymid,btn_inner_rect.x+(btn_inner_rect.w)/2,btn_inner_ybot);
		render_ctx.renderer.DrawLine(btn_inner_rect.x+btn_inner_rect.w,btn_ymid,btn_inner_rect.x+(btn_inner_rect.w)/2,btn_inner_ybot);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Enlight hovered button
		btn_rect.x += (TITLEBTN_N-titlebtn_hovered)*title_button_width;
		if (titlebtn_pressed != TITLEBTN_NONE) {
			if (titlebtn_pressed == titlebtn_hovered) {
				render_ctx.renderer.SetDrawColor(16,16,16,128);
				render_ctx.renderer.FillRect(btn_rect);
			}
		} else if (titlebtn_hovered != TITLEBTN_NONE) {
			render_ctx.renderer.SetDrawColor(255,255,255,128);
			render_ctx.renderer.FillRect(btn_rect);
		}
	}
}

void Arcollect::gui::set_fullscreen(bool fullscreen)
{
	if (fullscreen) {
		SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL_SetWindowFullscreen(window,0);
		// GNOME Shell workaround if the program is started in fullscreen mode
		// FIXME Check if it happen on Windows
		// FIXME Check if it happen on macOS
		SDL_SetWindowBordered(window,SDL_FALSE);
		SDL_SetWindowFullscreen(window,0);
	}
}
