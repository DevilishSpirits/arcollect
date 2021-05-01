#include "slideshow.hpp"
#include "artwork-collections.hpp"
Arcollect::gui::view_slideshow Arcollect::gui::background_slideshow;

void Arcollect::gui::update_background(Arcollect::db::artwork_id artid)
{
	std::shared_ptr<Arcollect::gui::artwork_collection> collection = std::make_shared<Arcollect::gui::artwork_collection_single>(artid);
	background_slideshow.set_collection(collection);
}
void Arcollect::gui::update_background(std::unique_ptr<SQLite3::stmt> &stmt)
{
	if (stmt->step() == SQLITE_ROW)
		update_background(stmt->column_int64(0));
}

void Arcollect::gui::update_background(void)
{
	std::unique_ptr<SQLite3::stmt> stmt;
	database->prepare("SELECT art_artid FROM artworks ORDER BY random() LIMIT 1;",stmt);
	update_background(stmt);
}
