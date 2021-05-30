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
#include "../db/artwork.hpp"
namespace Arcollect {
	namespace gui {
		/** Abtract artwork collection interface
		 *
		 * This interface separate the artwork listing logic with the display.
		 */
		class artwork_collection {
			public:
				class iterator_base {
					public:
						virtual iterator_base& operator++(void) = 0;
						virtual iterator_base& operator--(void) = 0;
						virtual iterator_base& operator+=(int count) {
							// Inneficient emulation
							while (count--)
								this->operator++();
							return *this;
						}
						virtual iterator_base& operator-=(int count) {
							// Inneficient emulation
							while (count--)
								this->operator--();
							return *this;
						}
						virtual bool operator==(const iterator_base& right) const = 0;
						virtual std::shared_ptr<db::artwork>& operator*(void) = 0;
						virtual iterator_base* copy(void) = 0;
						virtual ~iterator_base(void) = default;
				};
				virtual iterator_base *new_begin(void) = 0;
				virtual iterator_base *new_end(void) = 0;
				class iterator {
					private:
						iterator_base* base;
					public:
						iterator(iterator_base *base) : base(base) {}
						iterator(const iterator &base) : base(base.base->copy()) {}
						~iterator(void) {
							delete base;
						}
						iterator& operator++(void) {
							base->operator++();
							return *this;
						}
						iterator& operator--(void) {
							base->operator--();
							return *this;
						}
						iterator& operator+=(int count) {
							base->operator+=(count);
							return *this;
						}
						iterator& operator-=(int count) {
							base->operator-=(count);
							return *this;
						}
						bool operator==(const iterator& right) const {
							return base->operator==(*right.base);
						}
						bool operator!=(const iterator& right) const {
							return !(base->operator==(*right.base));
						}
						std::shared_ptr<db::artwork>& operator*(void) {
							return base->operator*();
						}
				};
				iterator begin(void) {
					return iterator(new_begin());
				}
				iterator end(void) {
					return iterator(new_end());
				}
				virtual ~artwork_collection(void) = default;
		};
	}
}
