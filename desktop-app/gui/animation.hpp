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
		namespace animation {
			/** Scrolling animation
			 *
			 * This template implement a scrolling animation. Not fancy but it works.
			 */
			template <typename T, Uint32 default_time_delta = 1000>
			struct scrolling {
				Uint32 time_start = 0;
				Uint32 time_end   = 0;
				T val_target;
				T val_origin;
				operator T(void) {
					if (time_now >= time_end)
						// If animation is done, don't do anything
						return val_target;
					else {
						animation_running = true;
						return val_origin + (val_origin-val_target)*(static_cast<int32_t>(time_start-time_now)/(float)static_cast<int32_t>(time_end-time_start));
					}
				}
				void set(const T new_target, const Uint32 time_delta) {
					if ((val_target == new_target) && (time_end <= time_now + time_delta))
						return; // Do nothing
					val_origin = *this;
					val_target = new_target;
					time_start = time_now;
					time_end   = time_now + time_delta;
					animation_running = true;
				}
				inline void operator=(const T new_target) {
					set(new_target,default_time_delta);
				}
				inline T operator+=(const T delta_target) {
					set(val_target+delta_target,default_time_delta);
					return val_target;
				}
				inline T operator-=(const T delta_target) {
					set(val_target-delta_target,default_time_delta);
					return val_target;
				}
				scrolling(void) = default;
				scrolling(const T value) : val_target(value) {};
			};
		}
	}
}
