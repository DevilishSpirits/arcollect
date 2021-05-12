#include "../db/filter.hpp"
#include "slideshow.hpp"
#include "artwork-collections.hpp"

static class background_vgrid: public Arcollect::gui::view_vgrid {
	Arcollect::gui::artwork_viewport *mousedown_viewport;
	bool event(SDL::Event &e) override {
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN: {
				mousedown_viewport = get_pointed({e.button.x,e.button.y});
			} return false;
			case SDL_MOUSEBUTTONUP: {
				if (mousedown_viewport && (mousedown_viewport == get_pointed({e.button.x,e.button.y})))
					Arcollect::gui::background_slideshow.set_collection_iterator(*(mousedown_viewport->iter));
				Arcollect::gui::modal_stack.pop_back();
			} return false;
			default:break;
		}
		return Arcollect::gui::view_vgrid::event(e);
	}
} background_vgrid;

static class background_slideshow: public Arcollect::gui::view_slideshow {
	bool event(SDL::Event &e) override {
		switch (e.type) {
			case SDL_KEYUP: {
				switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_ESCAPE: {
						background_vgrid.resize(rect);
						Arcollect::gui::modal_stack.push_back(background_vgrid);
					} break;
					default:break;
				}
			} break;
			default:break;
		};
		return Arcollect::gui::view_slideshow::event(e);
	}
} background_slideshow;

Arcollect::gui::view_slideshow &Arcollect::gui::background_slideshow = ::background_slideshow;

void Arcollect::gui::update_background(Arcollect::db::artwork_id artid)
{
	std::shared_ptr<Arcollect::gui::artwork_collection> collection = std::make_shared<Arcollect::gui::artwork_collection_single>(artid);
	background_slideshow.set_collection(collection);
	background_vgrid.set_collection(collection);
}
void Arcollect::gui::update_background(std::unique_ptr<SQLite3::stmt> &stmt, bool collection)
{
	if (collection) {
		std::shared_ptr<Arcollect::gui::artwork_collection> new_collection = std::make_shared<Arcollect::gui::artwork_collection_sqlite>(stmt);
		background_slideshow.set_collection(new_collection);
		background_vgrid.set_collection(new_collection);
	} else if (stmt->step() == SQLITE_ROW)
		update_background(stmt->column_int64(0));
}

void Arcollect::gui::update_background(bool collection)
{
	std::unique_ptr<SQLite3::stmt> stmt;
	database->prepare("SELECT art_artid FROM artworks WHERE "+db_filter::get_sql()+" ORDER BY random();",stmt);
	update_background(stmt,collection);
}
