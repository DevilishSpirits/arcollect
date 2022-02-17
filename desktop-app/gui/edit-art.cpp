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
#include "view-vgrid.hpp"
#include "rating-selector.hpp"
#include "window-borders.hpp"
#include <iostream>
#include <memory>

struct edit_artwork_confirm: public Arcollect::gui::menu {
	int last_width = -1;
	std::unique_ptr<Arcollect::gui::view> view;
	std::unique_ptr<Arcollect::gui::font::Renderable> confirm_renderable;
	bool button_press = false;
	
	bool event(SDL::Event &e, Arcollect::gui::modal::render_context render_ctx) override {
		// Pop the modal upon click
		switch (e.type) {
			case SDL_MOUSEBUTTONDOWN: {
				button_press = true;
			} break;
			case SDL_MOUSEBUTTONUP: {
				to_pop = button_press;
			} break;
			case SDL_KEYUP: {
				switch (e.key.keysym.scancode) {
					// Exit upon escpae
					case SDL_SCANCODE_ESCAPE: {
						to_pop = true;
					} break;
					default:break;
				}
			} break;
		}
		// Strip the top bar
		render_ctx.target.y += Arcollect::gui::window_borders::title_height;
		render_ctx.target.h -= Arcollect::gui::window_borders::title_height;
		if (confirm_renderable) {
			render_ctx.target.y += confirm_renderable->size().y;
			render_ctx.target.h -= confirm_renderable->size().y;
		}
		if (Arcollect::gui::menu::event(e,render_ctx))
			return to_pop & view->event(e,render_ctx);
		else return true;
	}
	void render(Arcollect::gui::modal::render_context render_ctx) override {
		SDL::Rect full_target = render_ctx.target;
		// Strip the top bar
		render_ctx.target.y += Arcollect::gui::window_borders::title_height;
		render_ctx.target.h -= Arcollect::gui::window_borders::title_height;
		// Check render cache
		if (last_width != render_ctx.target.w) {
			view->resize(render_ctx.target);
			Arcollect::gui::font::RenderConfig render_config;
			render_config.base_font_height *= 3;
			confirm_renderable = std::make_unique<Arcollect::gui::font::Renderable>(Arcollect::gui::font::Elements::build(Arcollect::i18n_desktop_app.edit_confirm_title),render_ctx.target.w,render_config);
			last_width = render_ctx.target.w;
		}
		render_ctx.target.y += confirm_renderable->size().y;
		render_ctx.target.h -= confirm_renderable->size().y;
		// Render the grid
		view->render(render_ctx);
		render_ctx.renderer.SetDrawColor({0,0,0,128});
		render_ctx.renderer.FillRect(full_target);
		// Render label
		confirm_renderable->render_tl(render_ctx.target.x + (render_ctx.target.w-confirm_renderable->size().x)/2,render_ctx.target.y-confirm_renderable->size().y);
		// Render the menu
		Arcollect::gui::menu::render(render_ctx);
	}
	// Perform deletion
	edit_artwork_confirm(std::shared_ptr<Arcollect::db::artwork_collection> collection, std::function<void(std::shared_ptr<Arcollect::db::artwork_collection>&)> action, const Arcollect::gui::font::Elements& confirm_label) {
		auto elements = Arcollect::gui::font::Elements::build();
		// Setup the view
		view = std::make_unique<Arcollect::gui::view_vgrid>();
		view->set_collection(collection);
		// Setup the popup menu
		anchor_top = anchor_left = anchor_bot = anchor_right = false;
		append_menu_item(std::make_shared<Arcollect::gui::menu_item_simple_label>(confirm_label,std::bind(action,collection)));
	}
};

struct on_set_rating {
	std::shared_ptr<Arcollect::db::artwork_collection> collection;
	void operator()(Arcollect::config::Rating rating)
	{
		using namespace Arcollect::gui;
		modal_back().to_pop = true;
		SDL::Color rating_color;
		switch (rating) {
			case Arcollect::config::Rating::RATING_NONE:rating_color = SDL::Color(128,255,128,255);break;
			case Arcollect::config::Rating::RATING_MATURE:rating_color = SDL::Color(128,128,255,255);break;
			case Arcollect::config::Rating::RATING_ADULT:rating_color = SDL::Color(255,128,128,255);break;
			default:rating_color = SDL::Color(255,255,255,255);break;
		}
		Arcollect::gui::modal_stack.emplace_back(std::make_unique<edit_artwork_confirm>(collection,[rating](std::shared_ptr<Arcollect::db::artwork_collection>& collection){
			collection->db_set_rating(rating);
		},Arcollect::i18n_desktop_app.edit_artwork_set_rating_confirm(rating,rating_color)));
	}
};

void Arcollect::gui::popup_edit_art_metadata(std::shared_ptr<Arcollect::db::artwork_collection>& collection)
{
	using namespace Arcollect::gui;
	const auto delete_elements = font::Elements::build(SDL::Color{255,0,0,255},i18n_desktop_app.edit_artwork_delete);
	const auto set_rating_elements = font::Elements::build(i18n_desktop_app.edit_artwork_set_rating);
	// Build GUI
	const std::vector<std::shared_ptr<menu_item>> menu_items = {
		std::make_shared<menu_item_simple_label>(delete_elements,[collection](){
			Arcollect::gui::modal_stack.emplace_back(std::make_unique<edit_artwork_confirm>(collection,[](std::shared_ptr<Arcollect::db::artwork_collection>& collection){
				collection->db_delete();
			},Arcollect::gui::font::Elements::build(SDL::Color{255,0,0,255},i18n_desktop_app.edit_artwork_delete_confirm)));
		}),
		std::make_shared<rating_selector_menu>(on_set_rating{collection},set_rating_elements),
	};
	const auto title_height = Arcollect::gui::window_borders::title_height;
	Arcollect::gui::menu::popup_context(menu_items,{title_height,title_height},true,false,false,true);
}
