#include "window-borders.hpp"
static bool display_bar = false;
const int Arcollect::gui::window_borders::title_height = 32;
const int Arcollect::gui::window_borders::resize_width = 4;
bool Arcollect::gui::window_borders::borderless;
extern SDL::Renderer *renderer;


static const int title_button_height  = Arcollect::gui::window_borders::title_height;
static const int title_button_width   = Arcollect::gui::window_borders::title_height;
enum TitleButton: int {
	TITLEBTN_CLOSE,
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
	if (borderless)
		SDL_SetWindowBordered(window,SDL_FALSE);
	return borderless;
}

bool Arcollect::gui::window_borders::event(SDL::Event &e)
{
	// TODO Proper cursor allocations
	// TODO Use X11 full cursor range
	SDL::Point cursor_position{e.motion.x,e.motion.y};
	static SDL_Cursor* cursor_normal = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	static SDL_Cursor* cursor_nwse   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	static SDL_Cursor* cursor_nesw   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	static SDL_Cursor* cursor_we     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	static SDL_Cursor* cursor_ns     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	
	if (Arcollect::gui::window_borders::borderless) {
		switch (e.type) {
			case SDL_MOUSEMOTION: {
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
					case SDL_HITTEST_NORMAL:
					case SDL_HITTEST_DRAGGABLE:
					default: {
						SDL_SetCursor(cursor_normal);
					} break;
				}
			} return true;
			case SDL_MOUSEBUTTONDOWN: {
				if (titlebtn_hovered != TITLEBTN_NONE)
					titlebtn_pressed = titlebtn_hovered;
			} return true;
			case SDL_MOUSEBUTTONUP: {
				if (titlebtn_pressed == titlebtn_hovered)
					switch (titlebtn_pressed) {
						case TITLEBTN_CLOSE: {
							// Generate a quit event
							SDL_Event event;
							event.type = SDL_QUIT;
							SDL_PushEvent(&event);
						} break;
					}
				titlebtn_pressed = TITLEBTN_NONE;
			} return true;
		}
	}
	return true;
}
void Arcollect::gui::window_borders::render(void)
{
	const int title_button_padding = Arcollect::gui::window_borders::title_height/3;
	if (display_bar) {
		// Get bar size and cursor position
		SDL::Point window_size;
		renderer->GetOutputSize(window_size);
		SDL::Point cursor_position;
		SDL_GetMouseState(&cursor_position.x,&cursor_position.y);
		// Draw background
		SDL::Rect title_bar{0,0,window_size.x,Arcollect::gui::window_borders::title_height};
		renderer->SetDrawColor(0,0,0,128);
		renderer->FillRect(title_bar);
		// Compute buttons rects
		SDL::Rect titlebtn_rects[] = {
			{window_size.x-title_button_width,0,title_button_width,title_button_height}, // Close button
		};
		
		renderer->SetDrawColor(255,255,255,192);
		// Draw close button (a cross)
		SDL::Point close_tl{titlebtn_rects[TITLEBTN_CLOSE].x + title_button_padding,titlebtn_rects[TITLEBTN_CLOSE].y + title_button_padding};
		SDL::Point close_br{titlebtn_rects[TITLEBTN_CLOSE].x + titlebtn_rects[TITLEBTN_CLOSE].w - title_button_padding,titlebtn_rects[TITLEBTN_CLOSE].y + titlebtn_rects[TITLEBTN_CLOSE].h - title_button_padding};
		renderer->DrawLine(close_tl,close_br);
		renderer->DrawLine(close_tl.x,close_br.y,close_br.x,close_tl.y);
		// Enlight hovered button
		if (titlebtn_pressed != TITLEBTN_NONE) {
			if (titlebtn_pressed == titlebtn_hovered) {
				renderer->SetDrawColor(16,16,16,128);
				renderer->FillRect(titlebtn_rects[titlebtn_hovered]);
			}
		} else if (titlebtn_hovered != TITLEBTN_NONE) {
			renderer->SetDrawColor(255,255,255,128);
			renderer->FillRect(titlebtn_rects[titlebtn_hovered]);
		}
	}
}
