#include "artwork-viewport.hpp"
#include "artwork-collection.hpp"
namespace Arcollect {
	namespace gui {
		// Generic view interface
		
		class view {
			protected:
				std::shared_ptr<gui::artwork_collection> collection;
			public:
				inline std::shared_ptr<gui::artwork_collection> get_collection(void) const {
					return collection;
				}
				/** Set the collection
				 */
				virtual void set_collection(std::shared_ptr<gui::artwork_collection> &new_collection) = 0;
				/** Called upon viewport resize
				 */
				virtual void resize(SDL::Rect rect) = 0;
				/** Render now
				 */
				virtual void render(void) = 0;
				/** React to event
				 */
				virtual void event(SDL::Event &e) = 0;
				virtual ~view(void) = default;
		};
		class view_slideshow: public view {
			/* README BEFORE READING CODE!!!
			 *
			 * `*--*collection_iterator` may seem dark to you :
			 * 	`*collection_iterator` dereference the `std::unique_ptr` getting an (peusdo-)iterator (pseudo because not C++ compliant).
			 * 	`--(*collection_iterator)` perform operator-- on the iterator.
			 * 	`*(--*collection_iterator)` return the pointed std::shared_ptr<artwork>
			 */
			private:
				// The bounding rect
				SDL::Rect rect;
				// The main artwork viewport
				artwork_viewport viewport;
				std::unique_ptr<artwork_collection::iterator> collection_iterator;
			public:
				void set_collection(std::shared_ptr<gui::artwork_collection> &new_collection) override;
				void resize(SDL::Rect rect) override;
				void render(void) override;
				/** Render some info in the bottom of the window
				 *
				 * Should be called right after render()
				 *
				 * The card is rendered with a flat transparent black panel in the picture
				 * bottom with white text draw on it.
				 *
				 * The box height is 20% of the rect height. Text height is a third of
				 * this value.
				 *
				 */
				void render_info_incard(void);
				void event(SDL::Event &e) override;
		};
	}
}
