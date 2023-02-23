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
#pragma once
#include "../sdl2-hpp/SDL.hpp"
#include "time.hpp"
namespace Arcollect {
	namespace gui {
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
			template <typename T, unsigned int default_time_delta = 200>
			struct scrolling {
				Arcollect::time_point time_start;
				Arcollect::time_point time_end;
				T val_target;
				T val_origin;
				operator T(void) {
					if (Arcollect::frame_time >= time_end)
						// If animation is done, don't do anything
						return val_target;
					else {
						animation_running = true;
						return val_origin + (val_origin-val_target)*((time_start-Arcollect::frame_time).count()/(float)(time_end-time_start).count());
					}
				}
				void skip_transition(void) {
					time_end = Arcollect::frame_time;
				}
				void set(const T new_target, const std::chrono::milliseconds time_delta) {
					if ((val_target == new_target) && (time_end <= Arcollect::frame_time + time_delta))
						return; // Do nothing
					val_origin = *this;
					val_target = new_target;
					time_start = Arcollect::frame_time;
					time_end   = Arcollect::frame_time + time_delta;
					animation_running = true;
				}
				inline void operator=(const T new_target) {
					set(new_target,std::chrono::milliseconds(default_time_delta));
				}
				inline T operator+=(const T delta_target) {
					set(val_target+delta_target,std::chrono::milliseconds(default_time_delta));
					return val_target;
				}
				inline T operator-=(const T delta_target) {
					set(val_target-delta_target,std::chrono::milliseconds(default_time_delta));
					return val_target;
				}
				scrolling(void) = default;
				scrolling(const T value) : val_target(value) {};
			};
		}
	}
}
