/* Arcollect -- An artwork collection managers
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
#include "edit-art.hpp"
#include "../i18n.hpp"
#include "menu.hpp"
#include "rating-selector.hpp"
#include "window-borders.hpp"
#include <iostream>

struct on_set_rating {
	std::shared_ptr<Arcollect::db::artwork_collection> collection;
	void operator()(Arcollect::config::Rating rating)
	{
		std::shared_ptr<Arcollect::db::artwork_collection> &arts_to_set = collection;
		using namespace Arcollect::gui;
		modal_back().to_pop = true;
		// TODO i18n
		font::Elements elements(U"Mark artworks as "s,14);
		switch (rating) {
			case Arcollect::config::Rating::RATING_NONE: {
				elements << SDL::Color(128,255,128,255) << U"Safe"sv;
			} break;
			case Arcollect::config::Rating::RATING_MATURE: {
				elements << SDL::Color(128,128,255,255) << U"Mature"sv;
			} break;
			case Arcollect::config::Rating::RATING_ADULT: {
				elements << SDL::Color(255,128,128,255) << U"Adult"sv;
			} break;
		}
		menu::popup_context({std::make_shared<menu_item_simple_label>(elements,[arts_to_set,rating](){
			arts_to_set->db_set_rating(rating);
		})},{0,Arcollect::gui::window_borders::title_height});
		// FIXME Dirty hack to counter the next pop
		menu::popup_context({std::make_shared<menu_item_simple_label>(U""s,[](){})},{0,-256});
	}
};

void Arcollect::gui::popup_edit_art_metadata(std::shared_ptr<Arcollect::db::artwork_collection>& collection)
{
	using namespace Arcollect::gui;
	const auto delete_elements = font::Elements::build(font::FontSize(14),SDL::Color{255,0,0,255},i18n_desktop_app.edit_artwork_delete);
	const auto set_rating_elements = font::Elements::build(font::FontSize(14),i18n_desktop_app.edit_artwork_set_rating);
	// Build GUI
	const std::vector<std::shared_ptr<menu_item>> menu_items = {
		std::make_shared<menu_item_simple_label>(delete_elements,[collection](){
			// TODO i18n
			Arcollect::gui::font::Elements really_delete_elements(U"I really want to delete the artworks"s,14);
			really_delete_elements.initial_color()  = {255,0,0,255};
			Arcollect::gui::menu::popup_context({
				std::make_shared<Arcollect::gui::menu_item_simple_label>(really_delete_elements,[collection]() {
					collection->db_delete();
				})
			},{0,Arcollect::gui::window_borders::title_height});
		}),
		std::make_shared<rating_selector_menu>(on_set_rating{collection},set_rating_elements),
	};
	const auto title_height = Arcollect::gui::window_borders::title_height;
	Arcollect::gui::menu::popup_context(menu_items,{title_height,title_height},true,false,false,true);
}
