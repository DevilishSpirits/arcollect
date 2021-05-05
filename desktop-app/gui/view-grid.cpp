#include "views.hpp"
void Arcollect::gui::view_vgrid::set_collection(std::shared_ptr<gui::artwork_collection> &new_collection)
{
	left_iter = std::make_unique<gui::artwork_collection::iterator>(new_collection->begin());
	right_iter = std::make_unique<gui::artwork_collection::iterator>(new_collection->begin());
	collection = new_collection;
}

void Arcollect::gui::view_vgrid::flush_layout(void)
{
	// Destroy current layout
	viewports.clear();
	// Reset scrolling
	left_iter = std::make_unique<gui::artwork_collection::iterator>(collection->begin());
	right_iter = std::make_unique<gui::artwork_collection::iterator>(collection->begin());
	left_y = 0;
	right_y = 0;
	// Force viewport regeneration
	do_scroll(0);
}
void Arcollect::gui::view_vgrid::resize(SDL::Rect rect)
{
	this->rect = rect;
	flush_layout();
}
void Arcollect::gui::view_vgrid::render(void)
{
	SDL::Point displacement{0,scroll_position};
	for (auto &lines: viewports)
		for (artwork_viewport &viewport: lines)
			viewport.render(displacement);
}
bool Arcollect::gui::view_vgrid::event(SDL::Event &e)
{
	switch (e.type) {
		case SDL_KEYDOWN: {
		} return false;
		case SDL_KEYUP: {
			switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_HOME: {
					do_scroll(-scroll_position);
				} break;
			}
		} return false;
		case SDL_MOUSEMOTION: {
			Arcollect::gui::artwork_viewport *viewport = Arcollect::gui::view_vgrid::get_pointed({e.motion.x,e.motion.y});
			//std::cerr << "Hovering " << (viewport ? viewport->artwork->title() : "none") << std::endl; 
		} return false;
		// Only called Arcollect::gui::background_slideshow
		case SDL_WINDOWEVENT: {
			switch (e.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				case SDL_WINDOWEVENT_RESIZED: {
					resize({0,0,e.window.data1,e.window.data2});
				} break;
				default: {
				} break;
			}
		} return true;
	}
	return true;
}

void Arcollect::gui::view_vgrid::do_scroll(int delta)
{
	auto end_iter = collection->end();
	scroll_position += delta;
	if (scroll_position < 0)
		scroll_position = 0;
	// Create left viewports if needed
	while (left_y < scroll_position)
		new_line_left(left_y - artwork_height - artwork_margin.y);
	// Drop left viewports if too much
	while (left_y > scroll_position + 2 * artwork_height) {
		viewports.pop_front();
		left_y += artwork_height + artwork_margin.y;
	}
	// Create right viewports if needed
	// NOTE! right_y is offset by minus one row
	while ((right_y < scroll_position + rect.h) && (*right_iter != end_iter))
		new_line_right(right_y);
	// Drop right viewports if too much
	while (right_y > scroll_position + rect.h + 2 * artwork_height) {
		viewports.pop_back();
		right_y -= artwork_height + artwork_margin.y;
	}
}

void Arcollect::gui::view_vgrid::new_line_left(int y)
{
	const auto begin_iter = collection->begin();
	int free_space = rect.w-2*artwork_margin.x;
	std::vector<artwork_viewport> &new_viewports = viewports.emplace_front();
	// Generate viewports
	while (*left_iter != begin_iter) {
		std::shared_ptr<db::artwork> artwork = **left_iter;
		if (!new_line_check_fit(free_space,y,new_viewports,artwork)) {
			// Rollback and break
			++*left_iter;
			break;
		} else --*left_iter;
	}
	// Place viewport horizontally
	if (new_viewports.size()) {
		new_line_place_horizontal_r(free_space,new_viewports);
		left_y -= artwork_height + artwork_margin.x;
	} else viewports.pop_front(); // Drop the line
}
void Arcollect::gui::view_vgrid::new_line_right(int y)
{
	auto end_iter = collection->end();
	int free_space = rect.w-2*artwork_margin.x;
	std::vector<artwork_viewport> &new_viewports = viewports.emplace_back();
	// Generate viewports
	while (*right_iter != end_iter) {
		std::shared_ptr<db::artwork> artwork = **right_iter;
		if (!new_line_check_fit(free_space,y,new_viewports,artwork)) {
			// Rollback and break
			--*right_iter;
			break;
		} else {++*right_iter;
		}
	}
	// Place viewport horizontally
	if (new_viewports.size()) {
		new_line_place_horizontal_l(free_space,new_viewports);
		right_y += artwork_height + artwork_margin.y;
	} else viewports.pop_back(); // Drop the line
}
bool Arcollect::gui::view_vgrid::new_line_check_fit(int &free_space, int y, std::vector<artwork_viewport> &new_viewports, std::shared_ptr<db::artwork> &artwork)
{
	// Compute width
	SDL::Point size;
	artwork->QuerySize(size);
	size.x *= artwork_height;
	size.x /= size.y;
	// Check for overflow
	if (size.x + artwork_margin.x < free_space) {
		// It fit !
		artwork_viewport& viewport = new_viewports.emplace_back();
		viewport.artwork = artwork;
		viewport.set_corners({artwork_margin.x,y,size.x,artwork_height});
		free_space -= size.x + artwork_margin.x;
	} else {
		// Would overflow
		if (new_viewports.size()) {
			// Rollback and break
			return false;
		} else {
			// This is an ultra large artwork !
			artwork_viewport& viewport = new_viewports.emplace_back();
			viewport.artwork = artwork;
			viewport.set_corners({0,y,free_space,artwork_height});
			// Remove all free_space
			free_space = 0;
			return true;
		}
	}
	return true;
}
void Arcollect::gui::view_vgrid::new_line_place_horizontal_l(int free_space, std::vector<artwork_viewport> &new_viewports)
{
	int spacing = artwork_margin.x + free_space / (new_viewports.size() > 1 ? new_viewports.size()-1 : new_viewports.size());
	int x = rect.x;
	for (artwork_viewport& viewport: new_viewports) {
		// Move corners
		viewport.corner_tl.x += x;
		viewport.corner_tr.x += x;
		viewport.corner_br.x += x;
		viewport.corner_bl.x += x;
		// Update x
		x += viewport.corner_tr.x - viewport.corner_tl.x + spacing;
	}
}
void Arcollect::gui::view_vgrid::new_line_place_horizontal_r(int free_space, std::vector<artwork_viewport> &new_viewports)
{
	int spacing = artwork_margin.x + free_space / (new_viewports.size() > 1 ? new_viewports.size()-1 : new_viewports.size());
	int x = rect.x + rect.w;
	for (artwork_viewport& viewport: new_viewports) {
		// Move corners
		viewport.corner_tl.x += x;
		viewport.corner_tr.x += x;
		viewport.corner_br.x += x;
		viewport.corner_bl.x += x;
		// Update x
		x -= viewport.corner_tr.x - viewport.corner_tl.x + spacing;
	}
}

Arcollect::gui::artwork_viewport *Arcollect::gui::view_vgrid::get_pointed(SDL::Point mousepos)
{
	mousepos.y -= left_y - scroll_position;
	// Locate the row
	const auto row_height = artwork_height + artwork_margin.y;
	auto pointed_row = mousepos.y / row_height;
	// Check if we are between rows
	if (mousepos.y % row_height > artwork_height)
		return NULL;
	// Check if we are on a row
	if ((pointed_row < 0)||(pointed_row >= viewports.size()))
		return NULL;
	// Check viewports
	auto rows_iter = viewports.begin();
	for (;pointed_row--;++rows_iter);
	
	for (auto &viewport: *rows_iter) {
		if ((viewport.corner_tl.x <= mousepos.x)&&(viewport.corner_tr.x >= mousepos.x))
			return &viewport;
	}
	return NULL;
}
