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
#include "../db/artwork-collection.hpp"
#include "modal.hpp"
namespace Arcollect {
	namespace gui {
		/** Generic view interface
		 *
		 * This interface allow to separate a collection of artworks and the way to
		 * display it.
		 */
		class view: public modal {
			protected:
				std::shared_ptr<db::artwork_collection> collection;
			public:
				// For convenience
				using artwork_collection = db::artwork_collection;
				inline std::shared_ptr<artwork_collection> get_collection(void) const {
					return collection;
				}
				/** Set the collection
				 */
				virtual void set_collection(std::shared_ptr<artwork_collection> &new_collection) = 0;
				/** Called upon viewport resize
				 */
				virtual void resize(SDL::Rect rect) = 0;
				virtual ~view(void) = default;
				
				static constexpr auto ARTWORK_TYPE_IMAGE   = Arcollect::db::download::ARTWORK_TYPE_IMAGE;
				static constexpr auto ARTWORK_TYPE_TEXT    = Arcollect::db::download::ARTWORK_TYPE_TEXT;
				static constexpr auto ARTWORK_TYPE_UNKNOWN = Arcollect::db::download::ARTWORK_TYPE_UNKNOWN;
		};
	}
}
