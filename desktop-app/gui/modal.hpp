#pragma once
#include "sdl2-hpp/SDL.hpp"
#include <functional>
#include <vector>
namespace Arcollect {
	namespace gui {
		/** Something that can be modal
		 *
		 * Arcollect UI use a stack of #Arcollect::gui::modal objects to deliver
		 * event and rendering.
		 */
		class modal {
			public:
				/** Handle an event
				 * \param e The event
				 * \return Weather propagate event to the next element in the stack.
				 *         True to propagate the event.
				 */
				virtual bool event(SDL::Event &e) = 0;
				/** Render the object
				 *
				 * Render the object on the screen
				 */
				virtual void render() = 0;
				virtual ~modal(void) = default;
		};
		
		extern std::vector<std::reference_wrapper<modal>> modal_stack;
	}
}
