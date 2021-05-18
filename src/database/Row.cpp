/*
 * Row.cpp
 *
 *  Created on: 11.12.2014
 *      Author: Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * Copyright (C) 2017 Geoffrey Mellar <mellar@gamma-kappa.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <ecs/database/Row.hpp>
#include <ecs/memory.hpp>
#include <utility>

using namespace ecs::db3;

ecs::db3::Row::Row() {
}

ecs::db3::Row::~Row() {
	clear();
}

void ecs::db3::Row::clear() {
	data.clear();
}

Row& ecs::db3::Row::operator <<(ecs::db3::types::cell_T::ptr_T ptr) {
	data.push_back(std::move(ecs::db3::types::cell_T::uniquePtr_T(ptr)));
	return *this;
}

Row& ecs::db3::Row::operator <<(ecs::db3::types::cell_T::uniquePtr_T &ptr) {
	data.push_back(std::move(ptr));
	return *this;
}

Row& ecs::db3::Row::operator <<(ecs::db3::types::cell_T::uniquePtr_T &&ptr) {
	data.push_back(std::move(ptr));
	return *this;
}

Row::ptr_T ecs::db3::createRow() {
	return new Row;
}

ecs::db3::types::cell_T& ecs::db3::Row::operator [](int columnNumber) {
	return *(data.at(columnNumber));
}

ecs::db3::types::cell_T& ecs::db3::Row::at(int columnNumber) {
	return *(data.at(columnNumber));
}


std::vector<ecs::db3::types::cell_T::uniquePtr_T>::size_type ecs::db3::Row::size() const {
	return data.size();
}

ecs::db3::RowBase::RowBase() {
}

ecs::db3::RowBase::~RowBase() {
}
