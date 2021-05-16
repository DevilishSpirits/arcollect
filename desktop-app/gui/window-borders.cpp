#include "window-borders.hpp"
#include "modal.hpp"
static bool display_bar = false;
const int Arcollect::gui::window_borders::title_height = 32;
const int Arcollect::gui::window_borders::resize_width = 4;
bool Arcollect::gui::window_borders::borderless;
extern SDL_Window    *window;
extern SDL::Renderer *renderer;


static const int title_button_height  = Arcollect::gui::window_borders::title_height;
static const int title_button_width   = Arcollect::gui::window_borders::title_height;
enum TitleButton: int {
	TITLEBTN_CLOSE,
	TITLEBTN_MAXIMIZE,
	TITLEBTN_FULLSCREEN,
	TITLEBTN_MINIMIZE,
	TITLEBTN_N,
	TITLEBTN_NONE = TITLEBTN_N,
};
static TitleButton titlebtn_hovered = TITLEBTN_NONE;
static TitleButton titlebtn_pressed = TITLEBTN_NONE;

static SDL_HitTestResult hit_test(SDL_Window *window, const SDL_Point *point, void* data)
{
	SDL::Point window_size;
	renderer->GetOutputSize(window_size);
	// TODO Check errors
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
	// Check for dragable area
	if ((point->y <= Arcollect::gui::window_borders::title_height)&&
	 (point->x < window_size.x-(title_button_width*TITLEBTN_N)))
		return SDL_HITTEST_DRAGGABLE;
	
	// No match
	return SDL_HITTEST_NORMAL;
}
bool Arcollect::gui::window_borders::init(SDL_Window *window)
{
	borderless = SDL_SetWindowHitTest(window,hit_test,NULL) == 0;
	if (borderless) {
		SDL_SetWindowBordered(window,SDL_FALSE);
		SDL_SetWindowMinimumSize(window,title_button_width*TITLEBTN_N,title_button_height);
	}
	return borderless;
}

bool Arcollect::gui::window_borders::event(SDL::Event &e)
{
	// TODO Proper cursor allocations
	// TODO Use X11 full cursor range
	SDL::Point cursor_position{e.motion.x,e.motion.y};
	if (Arcollect::gui::window_borders::borderless) {
		switch (e.type) {
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
					// Get window size
					SDL::Point window_size;
					renderer->GetOutputSize(window_size);
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
				if (titlebtn_pressed == titlebtn_hovered) {
					auto window_flags = SDL_GetWindowFlags(window);
					switch (titlebtn_pressed) {
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
									SDL_SetWindowFullscreen(window,0);
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
								SDL_SetWindowFullscreen(window,0);
							else SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN_DESKTOP);
						} break;
					}
				}
				titlebtn_pressed = TITLEBTN_NONE;
			} return cursor_position.y >= Arcollect::gui::window_borders::title_height;
		}
	}
	return true;
}
void Arcollect::gui::window_borders::render(void)
{
	const int title_button_padding = Arcollect::gui::window_borders::title_height/3;
	if (display_bar) {
		renderer->SetDrawBlendMode(SDL::BLENDMODE_BLEND);
		// Get bar size and cursor position
		SDL::Point window_size;
		renderer->GetOutputSize(window_size);
		SDL::Point cursor_position;
		SDL_GetMouseState(&cursor_position.x,&cursor_position.y);
		SDL::Rect title_bar{0,0,window_size.x,Arcollect::gui::window_borders::title_height};
		// Draw modal title
		SDL::Rect modal_bar{0,0,window_size.x-TITLEBTN_N*title_button_width,Arcollect::gui::window_borders::title_height};
		renderer->SetDrawColor(0,0,0,128);
		renderer->FillRect(modal_bar);
		modal_stack.back().get().render_titlebar(modal_bar,window_size.x);
		// Draw buttons background over title if it overflow
		SDL::Rect title_button_area{modal_bar.w,0,TITLEBTN_N*title_button_width,Arcollect::gui::window_borders::title_height};
		renderer->SetDrawColor(0,0,0,128);
		renderer->FillRect(title_button_area);
		// Compute buttons rects
		SDL::Rect btn_rect = {
			window_size.x - title_button_width,0,
			title_button_width,title_button_height
		};
		SDL::Rect btn_inner_rect = {
			btn_rect.x + title_button_padding,
			btn_rect.y + title_button_padding,
			btn_rect.w - 2*title_button_padding,
			btn_rect.h - 2*title_button_padding,
		};
		const int btn_inner_ybot = btn_inner_rect.y + btn_inner_rect.h;
		renderer->SetDrawColor(255,255,255,192);
		// Draw close button (a cross)
		SDL::Point close_tl{btn_inner_rect.x, btn_inner_rect.y};
		SDL::Point close_br{btn_inner_rect.x + btn_inner_rect.w, btn_inner_rect.y + btn_inner_rect.h};
		renderer->DrawLine(close_tl,close_br);
		renderer->DrawLine(close_tl.x,close_br.y,close_br.x,close_tl.y);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Draw maximize button (a square)
		renderer->DrawRect(btn_inner_rect);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Draw fullscreen button (a filled square)
		SDL::Rect fullscreen_rect = btn_inner_rect;
		renderer->DrawRect(fullscreen_rect);
		fullscreen_rect.x += 2;
		fullscreen_rect.y += 2;
		fullscreen_rect.w -= 4;
		fullscreen_rect.h -= 4;
		renderer->FillRect(fullscreen_rect);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Draw minimize button (a square)
		renderer->DrawLine(btn_inner_rect.x,btn_inner_ybot,btn_inner_rect.x+btn_inner_rect.w,btn_inner_ybot);
		btn_rect.x       -= title_button_width;
		btn_inner_rect.x -= title_button_width;
		// Enlight hovered button
		btn_rect.x += (TITLEBTN_N-titlebtn_hovered)*title_button_width;
		if (titlebtn_pressed != TITLEBTN_NONE) {
			if (titlebtn_pressed == titlebtn_hovered) {
				renderer->SetDrawColor(16,16,16,128);
				renderer->FillRect(btn_rect);
			}
		} else if (titlebtn_hovered != TITLEBTN_NONE) {
			renderer->SetDrawColor(255,255,255,128);
			renderer->FillRect(btn_rect);
		}
	}
}
