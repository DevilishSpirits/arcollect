#include "views.hpp"

void Arcollect::gui::view_slideshow::set_collection(std::shared_ptr<gui::artwork_collection> &new_collection)
{
	collection_iterator = std::make_unique<gui::artwork_collection::iterator>(new_collection->begin());
	collection = new_collection;
	viewport.artwork = **collection_iterator;
}
void Arcollect::gui::view_slideshow::resize(SDL::Rect rect)
{
	this->rect = rect;
	// Preserve aspect ratio
	SDL::Point art_size;
	viewport.artwork->QuerySize(art_size);
	int height_for_width = art_size.y*rect.w/art_size.x;
	if (height_for_width > rect.h) {
		// Perform width for height size
		int width_for_height = art_size.x*rect.h/art_size.y;
		rect.x += (rect.w-width_for_height)/2;
		rect.w = width_for_height;
	} else {
		// Perform height for width size
		rect.y += (rect.h-height_for_width)/2;
		rect.h = height_for_width;
	}
	// Set viewport
	viewport.set_corners(rect);
}
void Arcollect::gui::view_slideshow::render(void)
{
	viewport.render({0,0});
}
void Arcollect::gui::view_slideshow::event(SDL::Event &e)
{
	// STOP READING CODE!!! You might not understand some weird syntax.
	// There's a 'README BEFORE READING CODE!!!' in top Arcollect::gui::view_slideshow declaration.
	switch (e.type) {
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_RIGHT: {
					++*collection_iterator;
					if (*collection_iterator != collection->end()) {
						viewport.artwork = **collection_iterator;
						resize(rect);
					} else --*collection_iterator; // Rewind
				} break;
				case SDL_SCANCODE_LEFT: {
					if (*collection_iterator != collection->begin()) {
						viewport.artwork = *--*collection_iterator;
						resize(rect);
					}
				} break;
				case SDL_SCANCODE_HOME: {
					collection_iterator = std::make_unique<gui::artwork_collection::iterator>(collection->begin());
					viewport.artwork = **collection_iterator;
					resize(rect);
				} break;
				case SDL_SCANCODE_END: {
					collection_iterator = std::make_unique<gui::artwork_collection::iterator>(collection->end());
					viewport.artwork = *--*collection_iterator;
					resize(rect);
				} break;
			}
		} break;
	}
}

