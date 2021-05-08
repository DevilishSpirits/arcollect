#include "../sdl2-hpp/SDL.hpp"
namespace Arcollect {
	namespace gui {
		namespace window_borders {
			bool init(SDL_Window *window);
			extern bool borderless;
			extern const int title_height;
			extern const int resize_width;
			bool event(SDL::Event &e);
			void render(void);
		}
	}
}
