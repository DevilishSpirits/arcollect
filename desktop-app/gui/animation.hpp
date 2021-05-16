#pragma once
#include "../sdl2-hpp/SDL.hpp"
namespace Arcollect {
	namespace gui {
		extern Uint32 time_now;
		extern Uint32 time_framedelta;
		/** Animation running flag
		 *
		 * This boolean is set to true when an animation is running and that we
		 * should not wait for an event before drawing another frame.
		 */
		extern bool   animation_running;
	}
}
